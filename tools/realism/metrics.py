# SPDX-License-Identifier: Apache-2.0
"""Timbre metrics for the realism comparison (D-015): YIN f0, harmonic levels, centroid, attack, steady segment."""
from __future__ import annotations
import numpy as np


def _rms(x: np.ndarray) -> float:
    return float(np.sqrt(np.mean(np.square(x)))) if len(x) else 0.0


def f0_yin(x: np.ndarray, fs: float) -> float:
    """YIN f0 in Hz over the whole signal (threshold 0.1); 0.0 for silence (RMS < 1e-4)."""
    x = np.asarray(x, dtype=np.float64)
    cap = int(fs)
    if len(x) > cap:
        x = x[-cap:]
    if len(x) < 64 or _rms(x) < 1e-4:
        return 0.0
    tau_min = max(2, int(fs / 2500.0))
    tau_max = min(int(fs / 50.0) + 1, len(x) // 2)
    if tau_max <= tau_min + 2:
        return 0.0
    w = len(x) - tau_max
    m = 1 << int(np.ceil(np.log2(w + tau_max + 1)))
    a = np.fft.rfft(x[:w], m)
    b = np.fft.rfft(x[: w + tau_max], m)
    cc = np.fft.irfft(np.conj(a) * b, m)[: tau_max + 1]
    cs = np.concatenate([[0.0], np.cumsum(x * x)])
    t = np.arange(1, tau_max + 1)
    d = np.zeros(tau_max + 1)
    d[1:] = cs[w] + (cs[w + t] - cs[t]) - 2.0 * cc[1:]
    run = np.cumsum(d[1:])
    dn = np.ones(tau_max + 1)
    dn[1:] = np.where(run > 0, d[1:] * t / np.where(run > 0, run, 1.0), 1.0)
    best = 0
    i = tau_min
    while i < tau_max:
        if dn[i] < 0.1:
            while i + 1 < tau_max and dn[i + 1] < dn[i]:
                i += 1
            best = i
            break
        i += 1
    if best == 0:
        best = tau_min + int(np.argmin(dn[tau_min:tau_max]))
        if dn[best] > 0.3:
            return 0.0
    tau = float(best)
    if best > 1 and best + 1 <= tau_max:
        p, q, r = d[best - 1], d[best], d[best + 1]
        den = p - 2 * q + r
        if den > 0:
            tau += 0.5 * (p - r) / den
    return float(fs / tau)


def harmonic_levels_db(x: np.ndarray, fs: float, f0: float, n: int) -> np.ndarray:
    """Levels of harmonics 1..n in dB relative to the strongest, Hann-windowed FFT peak picking
    within +-3 % of h*f0. Raises ValueError for f0 <= 0 or n < 1."""
    if not f0 > 0 or n < 1:
        raise ValueError("f0 must be > 0 and n >= 1")
    x = np.asarray(x, dtype=np.float64)
    x = x - x.mean()
    win = np.hanning(len(x) + 1)[:-1]  # periodic Hann
    size = 1 << int(np.ceil(np.log2(len(x) * 4)))
    spec = np.abs(np.fft.rfft(x * win, size))
    bin_hz = fs / size
    amp = np.zeros(n)
    for h in range(1, n + 1):
        k0 = max(0, int(np.floor(0.97 * h * f0 / bin_hz)))
        k1 = min(len(spec) - 1, int(np.ceil(1.03 * h * f0 / bin_hz)))
        if k1 >= k0:
            amp[h - 1] = spec[k0 : k1 + 1].max()
    top = amp.max()
    out = np.full(n, -300.0)
    if top > 0:
        nz = amp > 0
        out[nz] = np.maximum(-300.0, 20 * np.log10(amp[nz] / top))
    return out


def spectral_centroid_hz(x: np.ndarray, fs: float) -> float:
    """Power-spectrum centroid over 20 Hz .. fs/2."""
    x = np.asarray(x, dtype=np.float64)
    x = x - x.mean()
    win = np.hanning(len(x) + 1)[:-1]
    size = 1 << int(np.ceil(np.log2(len(x))))
    p = np.abs(np.fft.rfft(x * win, size)) ** 2
    f = np.arange(len(p)) * fs / size
    sel = f >= 20.0
    den = p[sel].sum()
    return float((f[sel] * p[sel]).sum() / den) if den > 0 else -1.0


def _envelope(x: np.ndarray, fs: float) -> np.ndarray:
    win = max(1, int(round(0.010 * fs)))
    k = np.ones(win)
    num = np.convolve(np.square(x), k, mode="same")
    cnt = np.convolve(np.ones(len(x)), k, mode="same")
    return np.sqrt(num / cnt)


def attack_time_s(x: np.ndarray, fs: float) -> float:
    """Time from 10 % to 90 % of the maximum of the 10 ms RMS envelope."""
    x = np.asarray(x, dtype=np.float64)
    env = _envelope(x, fs)
    mx = env.max()
    if mx <= 0:
        return 0.0
    i10 = int(np.argmax(env >= 0.1 * mx))
    i90 = int(np.argmax(env >= 0.9 * mx))
    return float(max(0, i90 - i10) / fs)


def steady_segment(x: np.ndarray, fs: float) -> np.ndarray:
    """The segment where the 10 ms RMS envelope is >= 50 % of its maximum, trimmed by 100 ms at
    both ends; raises ValueError if shorter than 200 ms."""
    x = np.asarray(x, dtype=np.float64)
    env = _envelope(x, fs)
    idx = np.nonzero(env >= 0.5 * env.max())[0]
    if len(idx) == 0:
        raise ValueError("no steady segment")
    trim = int(round(0.1 * fs))
    seg = x[idx[0] + trim : idx[-1] + 1 - trim]
    if len(seg) < int(round(0.2 * fs)):
        raise ValueError("steady segment shorter than 200 ms")
    return seg


def compare(ref: np.ndarray, syn: np.ndarray, fs: float) -> dict:
    """Returns {"harm_mad_db", "centroid_ratio", "attack_ratio", "f0_ref", "f0_syn"} (see D-015)."""
    sr, ss = steady_segment(ref, fs), steady_segment(syn, fs)
    f0r, f0s = f0_yin(sr, fs), f0_yin(ss, fs)
    if f0r <= 0 or f0s <= 0:
        raise ValueError("f0 not found")
    nh = int(min(10, np.floor(0.45 * fs / max(f0r, f0s))))
    nh = max(nh, 2)
    lr = harmonic_levels_db(sr, fs, f0r, nh)[1:]
    ls = harmonic_levels_db(ss, fs, f0s, nh)[1:]
    ar, as_ = attack_time_s(np.asarray(ref, dtype=np.float64), fs), attack_time_s(np.asarray(syn, dtype=np.float64), fs)
    return {
        "harm_mad_db": float(np.mean(np.abs(lr - ls))),
        "centroid_ratio": spectral_centroid_hz(ss, fs) / spectral_centroid_hz(sr, fs),
        "attack_ratio": (as_ / ar) if ar > 0 else float("inf"),
        "f0_ref": f0r,
        "f0_syn": f0s,
    }
