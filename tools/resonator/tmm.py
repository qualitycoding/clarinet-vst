# SPDX-License-Identifier: Apache-2.0
"""Transfer-matrix model of a clarinet-like cylindrical bore with side holes, and modal extraction (D-009).
Plane waves; visco-thermal losses Gamma(s) = s/c + 1.044/R*sqrt(2 l_v/c)*sqrt(s) + 1.080 l_v/R^2
(Chaigne & Kergomard sec. 5.5 as used by Szwarcberg et al. 2025, C-031); unflanged radiation
Z_R = Zc (0.25 (kR)^2 + j 0.6133 kR); side holes as shunt impedances (open: radiating chimney with length
corrections; closed: closed chimney), series impedances neglected (Debut et al. 2005 sec. 3.3.2.2).
Impedances are dimensionless (Z / Zc of the main bore).
STUB: every function raises NotImplementedError."""
from __future__ import annotations
from dataclasses import dataclass, field
import numpy as np

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


def input_impedance(g: Geometry, f: np.ndarray) -> np.ndarray:
    """Dimensionless complex input impedance at the reed plane for frequencies f (Hz)."""
    raise NotImplementedError("input_impedance")


def cylinder_only_impedance(radius: float, length: float, f: np.ndarray, lossless: bool, radiation: bool) -> np.ndarray:
    """Closed-open cylinder (used for analytic verification)."""
    raise NotImplementedError("cylinder_only_impedance")


def petersen_resonator(fc: int) -> Geometry:
    """The simplified clarinet resonators of Petersen et al. 2020 Table 1 (fc in {1000, 1500, 2000}):
    a = 6.5 mm, h = 9.8 mm, l = 16.3 mm, b = 2.5/4.0/5.8 mm, L = 398.8/417.0/426.0 mm; ten open holes
    at L + l + 2 l k (k = 0..9); bore ends l after the last hole. Raises ValueError for other fc."""
    raise NotImplementedError("petersen_resonator")


def clarinet_geometry(written_midi: int, vent: str) -> Geometry:
    """D-009 virtual clarinet. vent 'none': bore of radius BORE_RADIUS, closed register hole, upstream
    length L solved (bisection, 1e-6 m) so the first impedance peak equals ET(written-2)*2^(25/1200), then
    the LATTICE of 5 open holes. vent 'register': the 'none' geometry of written-19 with the register hole
    open. vent 'altissimo': plain cylinder (no holes) with its length solved the same way for written.
    Raises ValueError for (written, vent) pairs outside D-009's table."""
    raise NotImplementedError("clarinet_geometry")


def modal_fit(f: np.ndarray, z: np.ndarray, n_modes: int) -> tuple[np.ndarray, np.ndarray]:
    """Poles s_n (rad/s, Re<0, sorted by Im) and complex residues C_n such that
    Z(w) ~= sum C_n/(jw - s_n) + conj(C_n)/(jw - conj(s_n)) (Colinot 2021 eq. 11). Peak-based pole
    initialisation, linear least squares for residues, then nonlinear refinement of all parameters.
    Raises ValueError if fewer than n_modes peaks."""
    raise NotImplementedError("modal_fit")


def modal_impedance(poles: np.ndarray, residues: np.ndarray, f: np.ndarray) -> np.ndarray:
    """Evaluate the modal sum of eq. 11."""
    raise NotImplementedError("modal_impedance")


def peak_frequencies(f: np.ndarray, z: np.ndarray, n: int) -> np.ndarray:
    """First n local maxima of |z| (frequency in Hz), parabolic interpolation on |z|."""
    raise NotImplementedError("peak_frequencies")
