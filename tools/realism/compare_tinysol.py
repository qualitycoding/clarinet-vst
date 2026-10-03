# SPDX-License-Identifier: Apache-2.0
"""CLI: render every TinySOL clarinet note with clar_render and write results.json (D-015, D-021).
Usage: python -m tools.realism.compare_tinysol --render BUILD/tools/render/clar_render --tinysol DIR --out FILE
results.json is a JSON list with one row per note (sorted by midi, dynamics); run metadata goes to FILE.meta.json."""
from __future__ import annotations

import argparse
import json
import math
import subprocess
import sys
import tempfile
from pathlib import Path

import numpy as np
import soundfile as sf
from scipy.signal import resample_poly

from tools.realism import metrics, tinysol

VELOCITY = {"pp": 0.25, "mf": 0.6, "ff": 0.95}  # D-015 dynamic mapping
FS = 44100
MAX_SECONDS = 4.0
RELEASE_BEFORE_END_S = 0.3
CONCERT_RANGE = (50, 94)


def _load_reference(path: Path) -> np.ndarray:
    x, fs = sf.read(str(path), dtype="float64", always_2d=True)
    x = x.mean(axis=1)
    if fs != FS:
        g = math.gcd(int(fs), FS)
        x = resample_poly(x, FS // g, int(fs) // g)
    return x


def _render(render: str, note: int, velocity: float, seconds: float, out: Path) -> np.ndarray:
    cmd = [render, "--note", str(note), "--velocity", f"{velocity}", "--seconds", f"{seconds:.3f}",
           "--fs", str(FS), "--out", str(out)]
    if seconds > 0.8:
        cmd += ["--release-at", f"{seconds - RELEASE_BEFORE_END_S:.3f}"]
    subprocess.run(cmd, check=True, capture_output=True)
    x, _ = sf.read(str(out), dtype="float64")
    return x


def _diagnostics(ref: np.ndarray, syn: np.ndarray, f0r: float, f0s: float) -> dict:
    """Extra per-note data used to calibrate the synthesis (not read by T-022b): harmonic levels (dB re the
    strongest of H1..H12, steady segment), attack times, steady-state RMS (dBFS) and the first 1.2 s of the
    10 ms RMS envelope (dB re its maximum, sampled every 20 ms)."""
    out: dict = {}
    for tag, x, f0 in (("ref", ref, f0r), ("syn", syn, f0s)):
        seg = metrics.steady_segment(x, FS)
        nh = int(min(12, math.floor(0.45 * FS / f0)))
        out[f"harm_{tag}_db"] = [round(float(v), 1) for v in metrics.harmonic_levels_db(seg, FS, f0, nh)]
        out[f"attack_{tag}_s"] = round(metrics.attack_time_s(np.asarray(x, dtype=np.float64), FS), 4)
        out[f"rms_{tag}_dbfs"] = round(float(20 * np.log10(max(np.sqrt(np.mean(seg ** 2)), 1e-9))), 1)
        env = metrics._envelope(np.asarray(x, dtype=np.float64), FS)
        step = int(0.02 * FS)
        e = env[: int(1.2 * FS) : step]
        out[f"env_{tag}_db"] = [round(float(20 * np.log10(max(v / max(env.max(), 1e-12), 1e-6))), 1) for v in e]
    return out


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(prog="compare_tinysol")
    ap.add_argument("--render", required=True, help="path to the built clar_render executable")
    ap.add_argument("--tinysol", required=True, help="TinySOL directory (downloaded there if absent)")
    ap.add_argument("--out", required=True, help="results.json to write")
    args = ap.parse_args(argv)

    root = Path(args.tinysol)
    if not (root / tinysol.METADATA).exists():
        tinysol.download(root)
    notes = tinysol.clarinet_notes(root)

    rows: list[dict] = []
    excluded = {"pitch_convention": 0, "out_of_range": 0, "analysis_failed": 0}
    with tempfile.TemporaryDirectory() as tmp:
        wav = Path(tmp) / "syn.wav"
        for n in notes:
            ref = _load_reference(n.path)
            try:
                f0 = metrics.f0_yin(metrics.steady_segment(ref, FS), FS)
            except ValueError:
                excluded["analysis_failed"] += 1
                continue
            if f0 <= 0:
                excluded["analysis_failed"] += 1
                continue
            concert = int(round(69 + 12 * math.log2(f0 / 440.0)))      # D-021: pitch from the measured f0
            offset = concert - n.pitch_id
            if offset not in (0, -2):
                excluded["pitch_convention"] += 1
                continue
            if not CONCERT_RANGE[0] <= concert <= CONCERT_RANGE[1]:
                excluded["out_of_range"] += 1
                continue
            seconds = min(len(ref) / FS, MAX_SECONDS)
            syn = _render(args.render, concert, VELOCITY[n.dynamics], seconds, wav)
            try:
                res = metrics.compare(ref[: int(seconds * FS)], syn, FS)
                centroid_syn = metrics.spectral_centroid_hz(metrics.steady_segment(syn, FS), FS)
            except ValueError:
                excluded["analysis_failed"] += 1
                continue
            row = {"midi": concert, "dynamics": n.dynamics, "harm_mad_db": res["harm_mad_db"],
                   "centroid_ratio": res["centroid_ratio"], "attack_ratio": res["attack_ratio"],
                   "centroid_syn_hz": centroid_syn, "f0_ref": res["f0_ref"], "f0_syn": res["f0_syn"],
                   "pitch_offset": offset}
            row.update(_diagnostics(ref[: int(seconds * FS)], syn, res["f0_ref"], res["f0_syn"]))
            rows.append(row)
    rows.sort(key=lambda r: (r["midi"], ("pp", "mf", "ff").index(r["dynamics"])))
    out = Path(args.out)
    out.write_text(json.dumps(rows, indent=1) + "\n")
    out.with_suffix(".meta.json").write_text(json.dumps(
        {"notes_in_metadata": len(notes), "rows": len(rows), "excluded": excluded}, indent=1) + "\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
