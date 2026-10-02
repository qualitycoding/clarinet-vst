// SPDX-License-Identifier: Apache-2.0
// STUB (S-000): replaced during implementation. Non-noexcept functions throw NotImplemented;
// noexcept functions return the sentinel documented in the header (plan/DECISIONS.md D-012).
#include "clar/Analysis.h"
#include "clar/Errors.h"
#include <limits>
namespace clar {
double estimateF0(std::span<const float>, double) noexcept { return -1.0; }
Regime classifyRegime(std::span<const float>, double, double) noexcept { return Regime::Other; }
std::vector<double> harmonicLevelsDb(std::span<const float>, double, double, int) { throw NotImplemented("harmonicLevelsDb"); }
double spectralCentroidHz(std::span<const float>, double) noexcept { return -1.0; }
double cents(double, double) noexcept { return std::numeric_limits<double>::quiet_NaN(); }
}
