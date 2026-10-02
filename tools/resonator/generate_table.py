# SPDX-License-Identifier: Apache-2.0
"""CLI: python -m tools.resonator.generate_table --out data/clarinet_resonators.json  (D-009).
Generates the 45-entry modal resonator table: written 52..70 'none', 71..84 'register', 85..96 'altissimo'.
Deterministic: fixed grids, no randomness, sorted keys, 6 significant digits."""
from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path

import numpy as np

from tools.resonator import tmm

NL_MODE1_BW_FACTOR = 10.0   # D-009 register-hole nonlinear-loss emulation (C-027, spikes C3/C4/C9)
MAX_MODES = 24
MIN_MODES = 2
GRID_STEP_HZ = 0.5


def _vent_for(written: int) -> str:
    if 52 <= written <= 70:
        return "none"
    if 71 <= written <= 84:
        return "register"
    return "altissimo"


def _r6(x: float) -> float:
    return float(f"{x:.6g}")


def entry_for(written: int) -> dict:
    vent = _vent_for(written)
    geom = tmm.clarinet_geometry(written, vent)
    probe = np.arange(20.0, 6000.0, GRID_STEP_HZ)
    zp = tmm.input_impedance(geom, probe)
    f1 = float(tmm.peak_frequencies(probe, zp, 1)[0])
    fmax = max(4000.0, 7.5 * f1)
    f = np.arange(20.0, fmax, GRID_STEP_HZ)
    z = tmm.input_impedance(geom, f)
    peaks = tmm._local_max(np.abs(z))
    n = min(MAX_MODES, len(peaks))
    if n < MIN_MODES:
        raise RuntimeError(f"written {written} ({vent}): only {n} impedance peaks")
    if len(peaks) > MAX_MODES:  # fit only up to just beyond the last retained peak
        keep = f <= f[peaks[MAX_MODES - 1]] * 1.05
        f, z = f[keep], z[keep]
    s, c = tmm.modal_fit(f, z, n)
    if vent == "register":      # D-009 nonlinear-loss emulation on the first (open-hole) resonance
        s = s.copy()
        s[0] = complex(s[0].real * NL_MODE1_BW_FACTOR, s[0].imag)
    modes = [{"re_s": _r6(a.real), "im_s": _r6(a.imag), "re_c": _r6(b.real), "im_c": _r6(b.imag)}
             for a, b in zip(s, c)]
    return {"written": written, "vent": vent, "tuning_scale": 1.0, "modes": modes}


def _git_sha() -> str:
    try:
        return subprocess.check_output(["git", "rev-parse", "HEAD"], stderr=subprocess.DEVNULL,
                                       cwd=Path(__file__).resolve().parents[2]).decode().strip()
    except Exception:
        return "unknown"


def render(entries: list[dict]) -> str:
    prov = {"generator": "tools/resonator/generate_table.py", "git": _git_sha(), "geometry": "D-009",
            "nl_mode1_bw_factor": NL_MODE1_BW_FACTOR}
    out = ['{', ' "entries": [']
    for i, e in enumerate(entries):
        out.append('  {"written": %d, "vent": "%s", "tuning_scale": %s, "modes": [' % (e["written"], e["vent"], repr(e["tuning_scale"])))
        out.append(",\n".join("   " + json.dumps(m) for m in e["modes"]) + "]}" + ("," if i + 1 < len(entries) else ""))
    out.append(' ],')
    out.append(' "instrument": "Bb soprano clarinet",')
    out.append(' "provenance": %s,' % json.dumps(prov, sort_keys=True))
    out.append(' "schema": "clarinet-vst/resonators@1",')
    out.append(' "units": {"pole": "rad/s", "residue": "Z/Zc"}')
    out.append('}')
    return "\n".join(out) + "\n"


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--out", required=True, type=Path)
    args = ap.parse_args(argv)
    entries = [entry_for(w) for w in range(52, 97)]
    text = render(entries)
    json.loads(text)  # sanity: must be valid JSON
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(text, encoding="utf-8")
    print(f"wrote {args.out} ({len(entries)} entries, {len(text)} bytes)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
