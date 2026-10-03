# FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
# SPDX-License-Identifier: Apache-2.0
"""T-022a (unit, code verification of the realism metrics, D-015). Synthetic signals with known answers."""
import numpy as np
import pytest
from tools.realism import metrics as M

FS = 48000


def tone(f0, amps, seconds=1.0, fs=FS):
    t = np.arange(int(seconds * fs)) / fs
    return 0.5 * sum(a * np.sin(2 * np.pi * f0 * (h + 1) * t) for h, a in enumerate(amps))


@pytest.mark.parametrize("f0", [138.59, 185.0, 440.0, 880.0])
def test_f0(f0):
    # YIN resolves < 1 cent on 1 s harmonic tones; allow 2 cents.
    assert abs(1200 * np.log2(M.f0_yin(tone(f0, [1, .5, .3]), FS) / f0)) < 2.0


def test_f0_silence():
    assert M.f0_yin(np.zeros(FS), FS) == 0.0


def test_harmonic_levels():
    L = M.harmonic_levels_db(tone(200.0, [1.0, 0.5, 0.25]), FS, 200.0, 3)
    np.testing.assert_allclose(L, [0.0, -6.0206, -12.0412], atol=0.1)
    with pytest.raises(ValueError):
        M.harmonic_levels_db(tone(200.0, [1.0]), FS, 0.0, 3)


def test_centroid():
    # powers 1, .25, .0625 at 200/400/600 Hz -> 257.14 Hz; window leakage < 1 %.
    assert M.spectral_centroid_hz(tone(200.0, [1.0, 0.5, 0.25]), FS) == pytest.approx(257.14, rel=0.01)


def test_attack_time_linear_ramp():
    x = tone(440.0, [1.0], seconds=1.0)
    env = np.minimum(np.arange(len(x)) / (0.1 * FS), 1.0)   # 100 ms linear ramp
    # 10 %..90 % of a linear ramp = 80 ms; 10 ms RMS window smears by < 10 ms.
    assert M.attack_time_s(x * env, FS) == pytest.approx(0.080, abs=0.010)


def test_steady_segment():
    x = tone(440.0, [1.0], seconds=1.0)
    seg = M.steady_segment(x, FS)
    assert 0.7 * FS <= len(seg) <= 0.85 * FS
    with pytest.raises(ValueError):
        M.steady_segment(tone(440.0, [1.0], seconds=0.25), FS)


def test_compare_identity_and_difference():
    a = tone(220.0, [1, .6, .4, .3, .2, .15, .1, .08, .06, .05])
    r = M.compare(a, a, FS)
    assert r["harm_mad_db"] == pytest.approx(0.0, abs=1e-6)
    assert r["centroid_ratio"] == pytest.approx(1.0, abs=1e-6)
    b = tone(220.0, [1, .3, .2, .15, .1, .075, .05, .04, .03, .025])   # harmonics 2..10 halved: -6.02 dB
    assert M.compare(a, b, FS)["harm_mad_db"] == pytest.approx(6.02, abs=0.2)
