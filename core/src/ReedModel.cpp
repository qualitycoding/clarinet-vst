// SPDX-License-Identifier: Apache-2.0
// Single-reed exciter, Colinot et al. 2021 eqs. (5)-(10) = Petersen et al. 2020 eqs. (7)-(9) (C-006, C-026).
#include "clar/ReedModel.h"
#include <cmath>
#include <numbers>

namespace clar {
namespace {
double sign(double v) noexcept { return v > 0.0 ? 1.0 : (v < 0.0 ? -1.0 : 0.0); }
} // namespace

ReedParams clarinetReed() noexcept {
    ReedParams p;
    p.omegaR = 2.0 * std::numbers::pi * 1500.0; // Petersen 2020 Table 2: f_r = 1500 Hz
    p.qR = 0.4;
    p.Kc = 100.0;
    p.eta = 0.01;
    return p;
}

double regularisedMinOpening(double x, double eta) noexcept {
    const double o = x + 1.0;
    return (o - std::sqrt(o * o + eta)) / 2.0;
}

double contactForce(double x, const ReedParams& p) noexcept {
    const double m = regularisedMinOpening(x, p.eta);
    return p.Kc * m * m;
}

double reedFlow(double x, double gamma, double pressure, double zeta, double eta) noexcept {
    const double o = x + 1.0;
    const double opening = (o + std::sqrt(o * o + eta)) / 2.0; // regularised max(x+1, 0)
    const double d = gamma - pressure;
    return zeta * opening * sign(d) * std::sqrt(std::sqrt(d * d + eta));
}

} // namespace clar
