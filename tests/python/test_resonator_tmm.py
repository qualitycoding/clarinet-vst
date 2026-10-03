# FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
# SPDX-License-Identifier: Apache-2.0
"""T-030 (unit, code verification of the resonator generator, D-009) and T-031 (generated table)."""
import json
import subprocess
import sys
from pathlib import Path
import numpy as np
import pytest
from tools.resonator import tmm

C0 = tmm.C0
ROOT = Path(__file__).resolve().parents[2]
# Colinot et al. 2021 Table 2 (D# fingering), CC BY 4.0 — used only to verify modal_fit.
TABLE2_S = np.array([-17.59+1195j, -35.50+2483j, -65.30+3727j, -269.34+4405j,
                     -70.32+5153j, -166.0+6177j, -94.49+6749j, -116.5+7987j])
TABLE2_C = np.array([176.1, 470.5, 649.4, 328.7, 541.5, 224.9, 382.2, 409.9], dtype=complex)


def peaks(f, z, n):
    m = np.abs(z)
    idx = [i for i in range(1, len(m) - 1) if m[i] > m[i - 1] and m[i] >= m[i + 1]]
    return np.array([f[i] for i in idx[:n]]), np.array([m[i] for i in idx[:n]])


def test_closed_open_cylinder_lossless_matches_quarter_wave():
    f = np.linspace(20, 2000, 200001)
    z = tmm.cylinder_only_impedance(0.0075, 0.5, f, lossless=True, radiation=False)
    expected = np.array([(2 * n - 1) * C0 / (4 * 0.5) for n in (1, 2, 3)])
    # grid step 0.0099 Hz -> analytic resonances recovered to 0.01 %
    np.testing.assert_allclose(peaks(f, z, 3)[0], expected, rtol=1e-4)


def test_cylinder_radiation_end_correction():
    f = np.linspace(20, 600, 58001)
    z = tmm.cylinder_only_impedance(0.0075, 0.5, f, lossless=True, radiation=True)
    # unflanged low-frequency end correction 0.6133 r (Levine & Schwinger); 0.5 % covers ka>0 drift
    assert peaks(f, z, 1)[0][0] == pytest.approx(C0 / (4 * (0.5 + 0.6133 * 0.0075)), rel=5e-3)


@pytest.mark.parametrize("fc", [1000, 1500, 2000])
def test_petersen_resonators_first_peak_and_cutoff(fc):
    # Petersen et al. 2020 Table 1 (C-030): all three resonators have f1 = 185 Hz with all holes open.
    # Spike C10: 185.2 / 184.8 / 184.6 Hz -> 1 % tolerance. Cutoff detected as the first peak lower than
    # 30 % of its predecessor: spike 1105 / 1512 / 1898 Hz -> +-15 % of the design cutoff.
    f = np.linspace(20, 5000, 49801)
    fp, mp = peaks(f, tmm.input_impedance(tmm.petersen_resonator(fc), f), 40)
    assert fp[0] == pytest.approx(185.0, rel=0.01)
    det = next(fp[i] for i in range(1, len(fp)) if mp[i] < 0.3 * mp[i - 1])
    assert det == pytest.approx(fc, rel=0.15)
    with pytest.raises(ValueError):
        tmm.petersen_resonator(1234)


def test_modal_impedance_single_mode_closed_form():
    s = np.array([-20 + 1200j]); c = np.array([150 + 10j]); f = np.array([100.0, 191.0])
    w = 2 * np.pi * f
    expect = c[0] / (1j * w - s[0]) + np.conj(c[0]) / (1j * w - np.conj(s[0]))
    np.testing.assert_allclose(tmm.modal_impedance(s, c, f), expect, rtol=1e-12)


def test_modal_fit_recovers_colinot_table2():
    f = np.linspace(20, 1400, 20000)
    z = tmm.modal_impedance(TABLE2_S, TABLE2_C, f)
    s, c = tmm.modal_fit(f, z, 8)
    np.testing.assert_allclose(s.imag, TABLE2_S.imag, rtol=1e-3)
    np.testing.assert_allclose(s.real, TABLE2_S.real, rtol=0.05)
    np.testing.assert_allclose(np.abs(c), np.abs(TABLE2_C), rtol=0.05)


def test_clarinet_geometry_closed_hole_is_an_odd_series_tuned_to_the_note():
    f = np.linspace(40, 2500, 49201)
    for w in (52, 58, 65, 70):
        fp, _ = peaks(f, tmm.input_impedance(tmm.clarinet_geometry(w, "none"), f), 2)
        target = 440 * 2 ** ((w - 2 - 69) / 12) * 2 ** (tmm.PITCH_OFFSET_CENTS / 1200)
        assert fp[0] == pytest.approx(target, rel=2e-3), w        # bisection to 1e-6 m + grid 0.05 Hz
        if w <= 65:
            assert 2.9 < fp[1] / fp[0] < 3.1, w                    # closed cylinder (C-025); spike C2: 3.015


def test_opening_the_register_hole_detunes_and_weakens_the_first_peak():
    # Debut et al. 2005; spike C2/C11: 177.1 -> 238.7 Hz for written G3, second peak shifts <= 1.9 %.
    f = np.linspace(40, 2500, 49201)
    for w in (55, 60, 65):
        fc_, mc = peaks(f, tmm.input_impedance(tmm.clarinet_geometry(w, "none"), f), 2)
        fo, mo = peaks(f, tmm.input_impedance(tmm.clarinet_geometry(w + 19, "register"), f), 2)
        assert fo[0] > 1.10 * fc_[0], w                           # spike C11: x1.35 / 1.23 / 1.13
        assert mo[0] < mc[0], w
        assert fo[1] == pytest.approx(fc_[1], rel=0.03), w         # second resonance nearly unchanged
    with pytest.raises(ValueError):
        tmm.clarinet_geometry(60, "register")                      # no clarion entry below written 71


def test_t031_generated_table_is_complete_and_deterministic(tmp_path):
    a, b = tmp_path / "a.json", tmp_path / "b.json"
    for out in (a, b):
        subprocess.run([sys.executable, "-m", "tools.resonator.generate_table", "--out", str(out)],
                       check=True, cwd=str(ROOT))
    assert a.read_bytes() == b.read_bytes()
    d = json.loads(a.read_text())
    assert d["schema"] == "clarinet-vst/resonators@1"
    assert d["provenance"]["nl_mode1_bw_factor"] == 10.0
    keys = {(e["written"], e["vent"]) for e in d["entries"]}
    assert len(d["entries"]) == 45
    assert keys == ({(w, "none") for w in range(52, 71)} | {(w, "register") for w in range(71, 85)}
                    | {(w, "altissimo") for w in range(85, 97)})
    for e in d["entries"]:
        assert 2 <= len(e["modes"]) <= 24, e["written"]
