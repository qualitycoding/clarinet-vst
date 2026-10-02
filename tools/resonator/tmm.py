# SPDX-License-Identifier: Apache-2.0
"""Transfer-matrix model of a clarinet-like cylindrical bore with side holes, and modal extraction (D-009).
Plane waves; visco-thermal losses Gamma(s) = s/c + 1.044/R*sqrt(2 l_v/c)*sqrt(s) + 1.080 l_v/R^2
(Chaigne & Kergomard sec. 5.5 as used by Szwarcberg et al. 2025, C-031); unflanged radiation
Z_R = Zc (0.25 (kR)^2 + j 0.6133 kR); side holes as shunt impedances (open: radiating chimney with length
corrections; closed: closed chimney), series impedances neglected (Debut et al. 2005 sec. 3.3.2.2).
Impedances are dimensionless (Z / Zc of the main bore).
The equations are those of research/spikes/tmm_spike.py (spike C1) and reed_spike.py (spike C2)."""
from __future__ import annotations

from dataclasses import dataclass, field
from functools import lru_cache

import numpy as np
from scipy.optimize import least_squares

C0 = 343.37      # m/s at 20 degC
RHO = 1.2041     # kg/m^3 at 20 degC
LV = 4e-8        # m, viscous boundary-layer length scale

BORE_RADIUS = 7.5e-3                                    # D-009 (C-028: Debut 2005, R = 7.5 mm)
LATTICE = {"radius": 4.0e-3 * 7.5 / 6.5, "chimney": 9.8e-3, "half_spacing": 16.3e-3, "count": 5}  # D-009
REGISTER_HOLE = {"position": 0.140, "radius": 1.55e-3, "chimney": 12.5e-3}                        # D-009, C-028
PITCH_OFFSET_CENTS = 25.0                               # D-009 initial f1 target above the note (C-013)


@dataclass(frozen=True)
class Hole:
    position: float          # m from the bore input (reed plane)
    radius: float            # m
    chimney: float           # m
    open: bool


@dataclass(frozen=True)
class Geometry:
    bore_radius: float
    length: float                        # m, input to open end
    holes: tuple[Hole, ...] = field(default_factory=tuple)
    lossless: bool = False
    radiation: bool = True


# ----------------------------------------------------------------------------- transfer matrices
def _gamma(f: np.ndarray, radius: float, lossless: bool) -> np.ndarray:
    s = 2j * np.pi * f
    if lossless:
        return s / C0
    return s / C0 + 1.044 / radius * np.sqrt(2 * LV / C0) * np.sqrt(s) + 1.080 * LV / radius ** 2


def _propagate(f, radius, length, z_end, lossless):
    """Input impedance of a cylinder of given length terminated by z_end (dimensional)."""
    if length <= 0.0:
        return z_end
    g = _gamma(f, radius, lossless)
    zc = RHO * C0 / (np.pi * radius ** 2)
    ch, sh = np.cosh(g * length), np.sinh(g * length)
    return (ch * z_end + zc * sh) / (sh / zc * z_end + ch)


def _radiation(f, radius):
    k = 2 * np.pi * f / C0
    zc = RHO * C0 / (np.pi * radius ** 2)
    return zc * (0.25 * (k * radius) ** 2 + 1j * 0.6133 * k * radius)


def _hole_shunt(f, hole: Hole, lossless):
    b, h = hole.radius, hole.chimney
    g = _gamma(f, b, lossless)
    zh = RHO * C0 / (np.pi * b ** 2)
    if hole.open:
        t = h + 0.6133 * b + 0.6 * b      # outer radiation + inner matching corrections (approx.)
        return zh * (np.tanh(g * t) + 0.25 * (2 * np.pi * f / C0 * b) ** 2)
    return zh / np.tanh(g * h)


def input_impedance(g: Geometry, f: np.ndarray) -> np.ndarray:
    """Dimensionless complex input impedance at the reed plane for frequencies f (Hz)."""
    f = np.asarray(f, dtype=np.float64)
    r = g.bore_radius
    zc = RHO * C0 / (np.pi * r ** 2)
    z = _radiation(f, r) if g.radiation else np.zeros(f.shape, dtype=np.complex128)
    pos = g.length
    for hole in sorted(g.holes, key=lambda hh: -hh.position):
        if not 0.0 < hole.position < g.length:
            raise ValueError("hole position outside the bore")
        z = _propagate(f, r, pos - hole.position, z, g.lossless)
        pos = hole.position
        z = 1.0 / (1.0 / z + 1.0 / _hole_shunt(f, hole, g.lossless))
    z = _propagate(f, r, pos, z, g.lossless)
    return z / zc


def cylinder_only_impedance(radius: float, length: float, f: np.ndarray, lossless: bool, radiation: bool) -> np.ndarray:
    """Closed-open cylinder (used for analytic verification)."""
    return input_impedance(Geometry(radius, length, (), lossless, radiation), f)


# ----------------------------------------------------------------------------- geometries
def petersen_resonator(fc: int) -> Geometry:
    """The simplified clarinet resonators of Petersen et al. 2020 Table 1 (fc in {1000, 1500, 2000}):
    a = 6.5 mm, h = 9.8 mm, l = 16.3 mm, b = 2.5/4.0/5.8 mm, L = 398.8/417.0/426.0 mm; ten open holes
    at L + l + 2 l k (k = 0..9); bore ends l after the last hole. Raises ValueError for other fc."""
    table = {1000: (398.8e-3, 2.5e-3), 1500: (417.0e-3, 4.0e-3), 2000: (426.0e-3, 5.8e-3)}
    if fc not in table:
        raise ValueError(f"fc must be one of {sorted(table)}")
    big_l, b = table[fc]
    a, h, l = 6.5e-3, 9.8e-3, 16.3e-3
    holes = tuple(Hole(big_l + l + 2 * l * k, b, h, True) for k in range(10))
    return Geometry(a, big_l + l + 2 * l * 9 + l, holes)


def _lattice_geometry(upstream: float, register_open: bool | None) -> Geometry:
    """Bore with the register hole (closed/open, None = absent) and the D-009 lattice; `upstream` = L."""
    half = LATTICE["half_spacing"]
    holes = []
    if register_open is not None:
        holes.append(Hole(REGISTER_HOLE["position"], REGISTER_HOLE["radius"], REGISTER_HOLE["chimney"], register_open))
    for i in range(LATTICE["count"]):
        holes.append(Hole(upstream + half + 2 * half * i, LATTICE["radius"], LATTICE["chimney"], True))
    length = upstream + half + 2 * half * (LATTICE["count"] - 1) + half
    return Geometry(BORE_RADIUS, length, tuple(holes))


_COARSE = np.geomspace(40.0, 4000.0, 2500)


def _first_peak_hz(g: Geometry) -> float:
    """First local maximum of |Z| (parabolic refinement on a fine local grid); inf if none below 4 kHz."""
    m = np.abs(input_impedance(g, _COARSE))
    idx = np.nonzero((m[1:-1] > m[:-2]) & (m[1:-1] >= m[2:]))[0]
    if len(idx) == 0:
        return float("inf")
    f0 = _COARSE[idx[0] + 1]
    fine = np.linspace(f0 * 0.99, f0 * 1.01, 201)
    mf = np.abs(input_impedance(g, fine))
    j = int(np.argmax(mf))
    if 0 < j < len(fine) - 1:
        a, b, c = mf[j - 1], mf[j], mf[j + 1]
        den = a - 2 * b + c
        if den < 0:
            return float(fine[j] + 0.5 * (a - c) / den * (fine[1] - fine[0]))
    return float(fine[j])


def _solve_upstream(target_hz: float, build) -> float:
    """Bisection (1e-6 m) on the upstream length so that the first |Z| peak equals target_hz."""
    lo, hi = 0.01, 0.9
    while hi - lo > 1e-6:
        mid = 0.5 * (lo + hi)
        if _first_peak_hz(build(mid)) > target_hz:
            lo = mid
        else:
            hi = mid
    return 0.5 * (lo + hi)


def _target_hz(written_midi: int) -> float:
    return 440.0 * 2.0 ** ((written_midi - 2 - 69) / 12.0) * 2.0 ** (PITCH_OFFSET_CENTS / 1200.0)


@lru_cache(maxsize=None)
def _closed_length(written_midi: int) -> float:
    return _solve_upstream(_target_hz(written_midi), lambda L: _lattice_geometry(L, False))


@lru_cache(maxsize=None)
def _altissimo_length(written_midi: int) -> float:
    return _solve_upstream(_target_hz(written_midi), lambda L: Geometry(BORE_RADIUS, L))


def clarinet_geometry(written_midi: int, vent: str) -> Geometry:
    """D-009 virtual clarinet. vent 'none': bore of radius BORE_RADIUS, closed register hole, upstream
    length L solved (bisection, 1e-6 m) so the first impedance peak equals ET(written-2)*2^(25/1200), then
    the LATTICE of 5 open holes. vent 'register': the 'none' geometry of written-19 with the register hole
    open. vent 'altissimo': plain cylinder (no holes) with its length solved the same way for written.
    Raises ValueError for (written, vent) pairs outside D-009's table."""
    if vent == "none" and 52 <= written_midi <= 70:
        return _lattice_geometry(_closed_length(written_midi), False)
    if vent == "register" and 71 <= written_midi <= 84:
        return _lattice_geometry(_closed_length(written_midi - 19), True)
    if vent == "altissimo" and 85 <= written_midi <= 96:
        return Geometry(BORE_RADIUS, _altissimo_length(written_midi))
    raise ValueError(f"no D-009 geometry for written={written_midi}, vent={vent!r}")


# ----------------------------------------------------------------------------- modal extraction
def modal_impedance(poles: np.ndarray, residues: np.ndarray, f: np.ndarray) -> np.ndarray:
    """Evaluate the modal sum of eq. 11."""
    w = 2j * np.pi * np.asarray(f, dtype=np.float64)
    poles = np.asarray(poles, dtype=np.complex128)
    residues = np.asarray(residues, dtype=np.complex128)
    out = np.zeros(w.shape, dtype=np.complex128)
    for s, c in zip(poles, residues):
        out += c / (w - s) + np.conj(c) / (w - np.conj(s))
    return out


def _local_max(m: np.ndarray) -> np.ndarray:
    return np.nonzero((m[1:-1] > m[:-2]) & (m[1:-1] >= m[2:]))[0] + 1


def peak_frequencies(f: np.ndarray, z: np.ndarray, n: int) -> np.ndarray:
    """First n local maxima of |z| (frequency in Hz), parabolic interpolation on |z|."""
    f = np.asarray(f, dtype=np.float64)
    m = np.abs(z)
    out = []
    for i in _local_max(m)[:n]:
        a, b, c = m[i - 1], m[i], m[i + 1]
        den = a - 2 * b + c
        d = 0.5 * (a - c) / den if den < 0 else 0.0
        out.append(f[i] + d * (f[i + 1] - f[i]))
    return np.array(out)


def _design(w, s):
    """Real-linear design matrix: columns for Re C_n then Im C_n."""
    a = np.stack([1 / (w - sn) + 1 / (w - np.conj(sn)) for sn in s], 1)
    b = np.stack([1j / (w - sn) - 1j / (w - np.conj(sn)) for sn in s], 1)
    return np.concatenate([a, b], 1)


def modal_fit(f: np.ndarray, z: np.ndarray, n_modes: int) -> tuple[np.ndarray, np.ndarray]:
    """Poles s_n (rad/s, Re<0, sorted by Im) and complex residues C_n such that
    Z(w) ~= sum C_n/(jw - s_n) + conj(C_n)/(jw - conj(s_n)) (Colinot 2021 eq. 11). Peak-based pole
    initialisation, linear least squares for residues, then nonlinear refinement of all parameters.
    Raises ValueError if fewer than n_modes peaks."""
    f = np.asarray(f, dtype=np.float64)
    z = np.asarray(z, dtype=np.complex128)
    m = np.abs(z)
    idx = _local_max(m)[:n_modes]
    if n_modes < 1 or len(idx) < n_modes:
        raise ValueError("fewer than n_modes impedance peaks")
    poles = []
    for i in idx:
        half = m[i] / np.sqrt(2.0)
        j = i
        while j > 0 and m[j] > half:
            j -= 1
        k = i
        while k < len(m) - 1 and m[k] > half:
            k += 1
        bw = max(f[k] - f[j], 2.0 * (f[1] - f[0]))
        poles.append(-np.pi * bw + 2j * np.pi * f[i])
    s0 = np.array(poles)
    w = 2j * np.pi * f
    weight = 1.0 / (m + 0.02 * m.max())
    mat = _design(w, s0)
    sel = np.concatenate([mat.real * weight[:, None], mat.imag * weight[:, None]])
    rhs = np.concatenate([z.real * weight, z.imag * weight])
    x, *_ = np.linalg.lstsq(sel, rhs, rcond=None)
    n = len(s0)
    c0 = x[:n] + 1j * x[n:]

    def unpack(p):
        return p[:n] + 1j * p[n:2 * n], p[2 * n:3 * n] + 1j * p[3 * n:]

    def resid(p):
        s, c = unpack(p)
        d = (modal_impedance(s, c, f) - z) * weight
        return np.concatenate([d.real, d.imag])

    def jac(p):
        s, c = unpack(p)
        cols = np.empty((2 * len(f), 4 * n))
        for k in range(n):
            d1, d2 = 1.0 / (w - s[k]), 1.0 / (w - np.conj(s[k]))
            dsig = c[k] * d1 ** 2 + np.conj(c[k]) * d2 ** 2
            dom = 1j * c[k] * d1 ** 2 - 1j * np.conj(c[k]) * d2 ** 2
            da = d1 + d2
            db = 1j * d1 - 1j * d2
            for col, v in ((k, dsig), (n + k, dom), (2 * n + k, da), (3 * n + k, db)):
                v = v * weight
                cols[:len(f), col] = v.real
                cols[len(f):, col] = v.imag
        return cols

    p0 = np.concatenate([s0.real, s0.imag, c0.real, c0.imag])
    lo = np.full(4 * n, -np.inf)
    hi = np.full(4 * n, np.inf)
    hi[:n] = -1e-3                                  # Re(s) < 0
    p0[:n] = np.minimum(p0[:n], -1e-2)
    sol = least_squares(resid, p0, jac=jac, bounds=(lo, hi), x_scale="jac", max_nfev=60)
    s, c = unpack(sol.x)
    order = np.argsort(s.imag)
    return s[order], c[order]
