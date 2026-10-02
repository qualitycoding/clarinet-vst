# SPDX-License-Identifier: Apache-2.0
"""Transfer-matrix model of a simplified saxophone and modal extraction (D-009).
Geometry: cylindrical mouthpiece (L_cyl = R1 / (3 tan phi)) + truncated cone, optional register side
hole at distance L1 from the cone input (Szwarcberg et al. 2025, Fig. 1). Visco-thermal losses and
unflanged radiation. Impedances are dimensionless (Z / Zc, Zc = rho c / (pi R1^2)).
STUB: every function raises NotImplementedError."""
from __future__ import annotations
from dataclasses import dataclass
import numpy as np

C0 = 343.37      # m/s at 20 degC
RHO = 1.2041     # kg/m^3 at 20 degC


@dataclass(frozen=True)
class Geometry:
    r1: float                 # cone input radius, m
    half_angle_deg: float     # cone half angle
    length: float             # cone length L, m
    hole_l1: float | None = None   # register hole distance from cone input, m (None = no hole)
    hole_radius: float = 0.0
    hole_chimney: float = 0.0
    hole_open: bool = False
    lossless: bool = False
    radiation: bool = True


def input_impedance(g: Geometry, f: np.ndarray) -> np.ndarray:
    """Dimensionless complex input impedance at the mouthpiece plane for frequencies f (Hz)."""
    raise NotImplementedError("input_impedance")


def cylinder_only_impedance(radius: float, length: float, f: np.ndarray, lossless: bool, radiation: bool) -> np.ndarray:
    """Closed-open cylinder (used for analytic verification)."""
    raise NotImplementedError("cylinder_only_impedance")


def modal_fit(f: np.ndarray, z: np.ndarray, n_modes: int) -> tuple[np.ndarray, np.ndarray]:
    """Poles s_n (rad/s, Re<0, sorted by Im) and residues C_n such that
    Z(w) ~= sum C_n/(jw - s_n) + conj(C_n)/(jw - conj(s_n)) (Colinot 2021 eq. 11). Least-squares
    refinement after peak-based initialisation. Raises ValueError if fewer than n_modes peaks."""
    raise NotImplementedError("modal_fit")


def modal_impedance(poles: np.ndarray, residues: np.ndarray, f: np.ndarray) -> np.ndarray:
    """Evaluate the modal sum of eq. 11."""
    raise NotImplementedError("modal_impedance")


def alto_geometry(written_midi: int, hole: str) -> Geometry:
    """Simplified alto: soprano parameters of Szwarcberg 2025 Table 1 scaled by D-009's factor; cone
    length solved so that the closed-hole first resonance equals the D-008 target for the note."""
    raise NotImplementedError("alto_geometry")
