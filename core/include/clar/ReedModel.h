// SPDX-License-Identifier: Apache-2.0
// Dimensionless single-reed exciter: Colinot et al. 2021 eqs. (4)-(10), identical in form to
// Petersen et al. 2020 eqs. (7)-(9) for the clarinet (C-006, C-026).
#pragma once

namespace clar {

struct ReedParams {
    double omegaR = 4224.0; ///< reed angular eigenfrequency, rad/s (default: Colinot 2021 Table 1)
    double qR     = 1.0;    ///< reed damping
    double Kc     = 100.0;  ///< lay contact stiffness
    double eta    = 1e-3;   ///< regularisation
};

/// Clarinet reed of Petersen et al. 2020 Table 2: f_r = 1500 Hz (omegaR = 2*pi*1500), q_r = 0.4,
/// K_c = 100, eta = 0.01 (C-026). The voice overrides qR per D-011. Stub sentinel: ReedParams{}.
ReedParams clarinetReed() noexcept;

/// Regularised min(x+1, 0) of eq. (6). Stub sentinel: NaN.
double regularisedMinOpening(double x, double eta) noexcept;

/// Contact force F_c(x+1) = Kc * min(x+1,0)^2, eq. (5) with eq. (6). Stub sentinel: NaN.
double contactForce(double x, const ReedParams& p) noexcept;

/// Reed-channel flow u, eq. (7) with regularisations (9),(10). Stub sentinel: NaN.
double reedFlow(double x, double gamma, double pressure, double zeta, double eta) noexcept;

} // namespace clar
