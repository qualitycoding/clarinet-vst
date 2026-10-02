# FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
# SPDX-License-Identifier: Apache-2.0
"""T-022b (integration, realism) — SC-4. Procedure frozen in D-015; thresholds are assumption A-012.
Requires env CLAR_RENDER (path to built clar_render) and TINYSOL_DIR (download target). The test
FAILS (never skips) when they are missing, so realism cannot be bypassed."""
import json
import os
from pathlib import Path
import numpy as np
from tools.realism import compare_tinysol

THRESH = {
    "harm_mad_db_mean": 6.0,     # mean over notes, per dynamic
    "harm_mad_db_max": 10.0,     # any single note
    "centroid_ratio": (0.8, 1.25),
    "attack_ratio": (0.5, 2.0),
    "fraction_required": 0.9,
}


def _results(tmp_path: Path) -> list[dict]:
    render = os.environ.get("CLAR_RENDER")
    tdir = os.environ.get("TINYSOL_DIR")
    assert render and Path(render).exists(), "set CLAR_RENDER to the built clar_render executable"
    assert tdir, "set TINYSOL_DIR (TinySOL is downloaded there if absent)"
    out = tmp_path / "results.json"
    assert compare_tinysol.main(["--render", render, "--tinysol", tdir, "--out", str(out)]) == 0
    rows = json.loads(out.read_text())
    assert len(rows) >= 80, "expected ~99 alto notes (33 pitches x 3 dynamics), minus TinySOL gaps"
    return rows


def test_realism_against_tinysol(tmp_path):
    rows = _results(tmp_path)
    for dyn in ("pp", "mf", "ff"):
        sub = [r for r in rows if r["dynamics"] == dyn]
        assert sub, dyn
        mad = np.array([r["harm_mad_db"] for r in sub])
        assert mad.mean() <= THRESH["harm_mad_db_mean"], (dyn, mad.mean())
        assert mad.max() <= THRESH["harm_mad_db_max"], (dyn, mad.max())
        lo, hi = THRESH["centroid_ratio"]
        ok = np.mean([lo <= r["centroid_ratio"] <= hi for r in sub])
        assert ok >= THRESH["fraction_required"], (dyn, "centroid", ok)
        lo, hi = THRESH["attack_ratio"]
        ok = np.mean([lo <= r["attack_ratio"] <= hi for r in sub])
        assert ok >= THRESH["fraction_required"], (dyn, "attack", ok)


def test_dynamics_brighten_like_a_real_sax(tmp_path):
    rows = _results(tmp_path)
    by = {}
    for r in rows:
        by.setdefault(r["midi"], {})[r["dynamics"]] = r["centroid_syn_hz"]
    trip = [d for d in by.values() if {"pp", "mf", "ff"} <= d.keys()]
    assert trip
    ok = np.mean([d["pp"] < d["mf"] < d["ff"] for d in trip])
    assert ok >= THRESH["fraction_required"], ok
