// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-007 (unit) — exciter equations, Colinot et al. 2021 eqs. (5)-(10); claim C-006.
// Expected values computed by hand from the closed forms (tolerance 1e-12 relative: closed form).
#include "clar/ReedModel.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <cmath>
using namespace clar;
using Catch::Approx;

TEST_CASE("T-007 regularised min and contact force", "[T-007][unit]") {
    const double eta = 1e-3;
    // x+1 = 0.5 (open): (0.5 - sqrt(0.25+1e-3))/2
    CHECK(regularisedMinOpening(-0.5, eta) == Approx((0.5 - std::sqrt(0.251)) / 2).epsilon(1e-12));
    // x+1 = -0.2 (beyond lay)
    const double m = (-0.2 - std::sqrt(0.04 + eta)) / 2;
    CHECK(regularisedMinOpening(-1.2, eta) == Approx(m).epsilon(1e-12));
    ReedParams p; // Kc = 100
    CHECK(contactForce(-1.2, p) == Approx(100.0 * m * m).epsilon(1e-12));
    CHECK(contactForce(0.0, p) < 1e-4); // reed at rest: negligible contact
}

TEST_CASE("T-007 reed-channel flow (Bernoulli with regularisation)", "[T-007][unit]") {
    const double eta = 1e-3, zeta = 0.6;
    // x = 0 (opening 1), gamma = 0.5, p = 0.1 -> u = zeta * max(1,0) * sign(0.4) * sqrt(sqrt(0.16+eta))
    const double mx = (1.0 + std::sqrt(1.0 + eta)) / 2;
    CHECK(reedFlow(0.0, 0.5, 0.1, zeta, eta) == Approx(zeta * mx * std::sqrt(std::sqrt(0.16 + eta))).epsilon(1e-12));
    // reversed pressure difference gives negative flow
    CHECK(reedFlow(0.0, 0.1, 0.5, zeta, eta) < 0.0);
    // closed channel (x = -1.5) -> flow nearly zero
    CHECK(std::abs(reedFlow(-1.5, 0.5, 0.0, zeta, eta)) < 1e-3);
}
