# SPDX-License-Identifier: Apache-2.0
"""Timbre metrics for the realism comparison (D-015). STUB: every function raises NotImplementedError."""
from __future__ import annotations
import numpy as np


def f0_yin(x: np.ndarray, fs: float) -> float:
    """YIN f0 in Hz over the whole signal (threshold 0.1); 0.0 for silence (RMS < 1e-4)."""
    raise NotImplementedError("f0_yin")


def harmonic_levels_db(x: np.ndarray, fs: float, f0: float, n: int) -> np.ndarray:
    """Levels of harmonics 1..n in dB relative to the strongest, Hann-windowed FFT peak picking
    within +-3 % of h*f0. Raises ValueError for f0 <= 0 or n < 1."""
    raise NotImplementedError("harmonic_levels_db")


def spectral_centroid_hz(x: np.ndarray, fs: float) -> float:
    """Power-spectrum centroid over 20 Hz .. fs/2."""
    raise NotImplementedError("spectral_centroid_hz")


def attack_time_s(x: np.ndarray, fs: float) -> float:
    """Time from 10 % to 90 % of the maximum of the 10 ms RMS envelope."""
    raise NotImplementedError("attack_time_s")


def steady_segment(x: np.ndarray, fs: float) -> np.ndarray:
    """The segment where the 10 ms RMS envelope is >= 50 % of its maximum, trimmed by 100 ms at
    both ends; raises ValueError if shorter than 200 ms."""
    raise NotImplementedError("steady_segment")


def compare(ref: np.ndarray, syn: np.ndarray, fs: float) -> dict:
    """Returns {"harm_mad_db": mean |dB difference| over harmonics 2..10 (both relative to their own
    strongest), "centroid_ratio": syn/ref on steady segments, "attack_ratio": syn/ref,
    "f0_ref": Hz, "f0_syn": Hz}."""
    raise NotImplementedError("compare")
