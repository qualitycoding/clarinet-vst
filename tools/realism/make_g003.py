# SPDX-License-Identifier: Apache-2.0
"""Builds the G-003 listening bundle (plan/GATES.md): demos, 12 blind real-vs-synthetic pairs, key, metrics summary.
Usage: python -m tools.realism.make_g003 --render clar_render --demo clar_demo --tinysol DIR --results results.json --out g003
Blind pairs: TinySOL clarinet mf notes (CC BY 4.0, trimmed to the steady segment, level-matched) next to a render of
the same pitch and dynamic; A/B order from random.Random(20261002). The answer key is base64 text so it is not read by accident."""
from __future__ import annotations

import argparse
import base64
import json
import math
import random
import subprocess
import sys
from pathlib import Path

import numpy as np
import soundfile as sf

from tools.realism import compare_tinysol as ct
from tools.realism import metrics, tinysol

SEED = 20261002
PAIRS = 12
CLIP_S = 2.0
TARGET_RMS_DBFS = -20.0
FADE_S = 0.03


def _prepare(x: np.ndarray) -> np.ndarray:
    seg = metrics.steady_segment(x, ct.FS)
    n = int(CLIP_S * ct.FS)
    start = max(0, (len(seg) - n) // 2)
    seg = seg[start : start + n]
    seg = seg * (10 ** (TARGET_RMS_DBFS / 20) / (np.sqrt(np.mean(seg**2)) + 1e-12))
    f = int(FADE_S * ct.FS)
    ramp = np.linspace(0.0, 1.0, f)
    seg[:f] *= ramp
    seg[-f:] *= ramp[::-1]
    return seg


def _summary(rows: list[dict]) -> str:
    lines = ["# G-003 objective metrics (T-022b procedure, frozen thresholds in tests/python/test_realism_vs_tinysol.py)", "",
             "| dynamic | notes | harmonic MAD mean (<= 6) | max (<= 10) | centroid ratio in [0.8,1.25] (>= 90 %) | attack ratio in [0.5,2] (>= 90 %) |",
             "|---|---|---|---|---|---|"]
    for d in ("pp", "mf", "ff"):
        s = [r for r in rows if r["dynamics"] == d]
        if not s:
            continue
        mad = np.array([r["harm_mad_db"] for r in s])
        cr = np.array([r["centroid_ratio"] for r in s])
        ar = np.array([r["attack_ratio"] for r in s])
        lines.append(f"| {d} | {len(s)} | {mad.mean():.1f} | {mad.max():.1f} | {100 * np.mean((cr >= .8) & (cr <= 1.25)):.0f} % | {100 * np.mean((ar >= .5) & (ar <= 2)):.0f} % |")
    by: dict[int, dict[str, float]] = {}
    for r in rows:
        by.setdefault(r["midi"], {})[r["dynamics"]] = r["centroid_syn_hz"]
    trip = [v for v in by.values() if {"pp", "mf", "ff"} <= v.keys()]
    if trip:
        lines += ["", f"Dynamics ordering (our centroid pp < mf < ff): {100 * np.mean([v['pp'] < v['mf'] < v['ff'] for v in trip]):.0f} % of {len(trip)} pitches (needs >= 90 %)."]
    low = [r for r in rows if 50 <= r["midi"] <= 59]
    if low:
        for d in ("pp", "mf", "ff"):
            s = [r for r in low if r["dynamics"] == d]
            if s:
                real = np.median([r["harm_ref_db"][2] - r["harm_ref_db"][1] for r in s])
                ours = np.median([r["harm_syn_db"][2] - r["harm_syn_db"][1] for r in s])
                lines.append(f"Odd-harmonic fingerprint, concert D3-B3 {d}: median (H3 - H2) real {real:.1f} dB, ours {ours:.1f} dB.")
    lines += ["", "Attack ratio: see TEST_CHALLENGE.md (the frozen criterion cannot be met by any plausible model)."]
    return "\n".join(lines) + "\n"


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(prog="make_g003")
    ap.add_argument("--render", required=True)
    ap.add_argument("--demo", required=True)
    ap.add_argument("--tinysol", required=True)
    ap.add_argument("--results", required=True)
    ap.add_argument("--out", required=True)
    args = ap.parse_args(argv)
    out = Path(args.out)
    (out / "demos").mkdir(parents=True, exist_ok=True)
    (out / "blind").mkdir(parents=True, exist_ok=True)

    subprocess.run([args.demo, str(out / "demos")], check=True)
    rows = json.loads(Path(args.results).read_text())
    (out / "results.json").write_text(json.dumps(rows, indent=1) + "\n")
    (out / "SUMMARY.md").write_text(_summary(rows))

    root = Path(args.tinysol)
    mf = [n for n in tinysol.clarinet_notes(root) if n.dynamics == "mf"]
    by_pitch: dict[int, tinysol.RefNote] = {}
    for n in mf:
        x = ct._load_reference(n.path)
        try:
            f0 = metrics.f0_yin(metrics.steady_segment(x, ct.FS), ct.FS)
        except ValueError:
            continue
        if f0 > 0:
            concert = int(round(69 + 12 * math.log2(f0 / 440.0)))
            if ct.CONCERT_RANGE[0] <= concert <= ct.CONCERT_RANGE[1]:
                by_pitch.setdefault(concert, n)
    rng = random.Random(SEED)
    pitches = rng.sample(sorted(by_pitch), min(PAIRS, len(by_pitch)))
    key = {}
    tmp = out / "_syn.wav"
    for i, p in enumerate(sorted(pitches), 1):
        real = _prepare(ct._load_reference(by_pitch[p].path))
        syn = _prepare(ct._render(args.render, p, ct.VELOCITY["mf"], 3.0, tmp))
        real_first = rng.random() < 0.5
        a, b = (real, syn) if real_first else (syn, real)
        sf.write(out / "blind" / f"pair-{i:02d}-A.wav", a, ct.FS, subtype="PCM_16")
        sf.write(out / "blind" / f"pair-{i:02d}-B.wav", b, ct.FS, subtype="PCM_16")
        key[f"pair-{i:02d}"] = {"concert_midi": p, "A": "real" if real_first else "synthetic", "B": "synthetic" if real_first else "real"}
    tmp.unlink(missing_ok=True)
    (out / "blind" / "key.json.b64").write_text(base64.b64encode(json.dumps(key, indent=1).encode()).decode() + "\n")
    (out / "ATTRIBUTION.txt").write_text(
        "The 'real' clips in blind/ are excerpts of TinySOL (Cella et al., CC BY 4.0, https://zenodo.org/record/3685367),\n"
        "trimmed to the steady segment, level-matched and faded (modified). Synthetic clips and demos: this repository.\n")
    print(f"G-003 bundle written to {out}: {len(key)} blind pairs")
    return 0


if __name__ == "__main__":
    sys.exit(main())
