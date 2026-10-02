// SPDX-License-Identifier: Apache-2.0
// STUB (S-000): replaced during implementation. Non-noexcept functions throw NotImplemented;
// noexcept functions return the sentinel documented in the header (plan/DECISIONS.md D-012).
#include "clar/ReedModel.h"
#include <limits>
namespace clar {
namespace { constexpr double nan = std::numeric_limits<double>::quiet_NaN(); }
ReedParams clarinetReed() noexcept { return ReedParams{}; }
double regularisedMinOpening(double, double) noexcept { return nan; }
double contactForce(double, const ReedParams&) noexcept { return nan; }
double reedFlow(double, double, double, double, double) noexcept { return nan; }
}
