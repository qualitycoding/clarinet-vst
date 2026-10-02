# FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
# SPDX-License-Identifier: Apache-2.0
"""T-030 (unit, code verification of the resonator generator, D-009) and T-031 (generated table)."""
import json
import subprocess
import sys
import numpy as np
import pytest
from tools.resonator import tmm

C0 = tmm.C0
# Colinot et al. 2021 Table 2 (D# fingering), CC BY 4.0
TABLE2_S = np.array([-17.59+1195j, -35.50+2483j, -65.30+3727j, -269.34+4405j,
                     -70.32+5153j, -166.0+6177j, -94.49+6749j, -116.5+7987j])
TABLE2_C = np.array([176.1, 470.5, 649.4, 328.7, 541.5, 224.9, 382.2, 409.9], dtype=complex)


def peaks(f, z, n):
    m = np.abs(z)
    idx = [i for i in range(1, len(m) - 1) if m[i] > m[i - 1] and m[i] >= m[i + 1]]
    return np.array([f[i] for i in idx[:n]])


def test_closed_open_cylinder_lossless_matches_quarter_wave():
    f = np.linspace(20, 2000, 200001)
    z = tmm.cylinder_only_impedance(0.008, 0.5, f, lossless=True, radiation=False)
    expected = np.array([(2 * n - 1) * C0 / (4 * 0.5) for n in (1, 2, 3)])
    # grid step 0.0099 Hz -> analytic resonances recovered to 0.01 %
    np.testing.assert_allclose(peaks(f, z, 3), expected, rtol=1e-4)


def test_cylinder_radiation_end_correction():
    f = np.linspace(20, 600, 58001)
    z = tmm.cylinder_only_impedance(0.008, 0.5, f, lossless=True, radiation=True)
    # unflanged low-frequency end correction 0.6133 r (Levine & Schwinger); 0.5 % covers ka>0 drift
    assert peaks(f, z, 1)[0] == pytest.approx(C0 / (4 * (0.5 + 0.6133 * 0.008)), rel=5e-3)


def test_conical_bore_resonances_nearly_harmonic():
    g = tmm.Geometry(r1=0.0069, half_angle_deg=1.74, length=0.9, lossless=True, radiation=False)
    f = np.linspace(20, 1500, 148001)
    p = peaks(f, tmm.input_impedance(g, f), 3)
    # complete cone of length L + apex: f_n = n c / (2 L_tot); equal-volume cylinder approximation
    # of the apex keeps the first resonances within 5 % (Colinot 2021 sec. 2.1.3, Benade).
    l_tot = 0.9 + 0.0069 / np.tan(np.radians(1.74))
    np.testing.assert_allclose(p, [n * C0 / (2 * l_tot) for n in (1, 2, 3)], rtol=0.05)


def test_modal_impedance_single_mode_closed_form():
    s = np.array([-20 + 1200j]); c = np.array([150 + 0j]); f = np.array([100.0, 191.0])
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


def test_open_register_hole_weakens_first_peak():
    f = np.linspace(20, 2500, 49601)
    for w in (74, 77, 80):
        z = tmm.input_impedance(tmm.alto_geometry(w, "body"), f)
        m = np.abs(z)
        idx = [i for i in range(1, len(m) - 1) if m[i] > m[i - 1] and m[i] >= m[i + 1]]
        assert m[idx[0]] < m[idx[1]], w   # Chen et al. 2009; Szwarcberg et al. 2025


def test_t031_generated_table_is_complete_and_deterministic(tmp_path):
    a, b = tmp_path / "a.json", tmp_path / "b.json"
    for out in (a, b):
        subprocess.run([sys.executable, "-m", "tools.resonator.generate_table", "--out", str(out)],
                       check=True, cwd=str(tmm.__file__).rsplit("/tools/", 1)[0])
    assert a.read_bytes() == b.read_bytes()
    d = json.loads(a.read_text())
    assert d["schema"] == "clarinet-vst/resonators@1"
    assert len(d["entries"]) == 33
    assert {e["written"] for e in d["entries"]} == set(range(58, 91))
