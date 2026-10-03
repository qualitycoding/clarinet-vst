// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-027 (unit, code verification of the measurement tools used by every audio test).
// Synthetic signals with known answers; tolerances from analysis resolution stated per check.
#include "clar/Analysis.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <cmath>
#include <numbers>
#include <vector>
using namespace clar;

static std::vector<float> harmonics(double fs, double f0, std::vector<double> amps, double seconds = 1.0) {
    std::vector<float> x(static_cast<size_t>(fs * seconds));
    for (size_t n = 0; n < x.size(); ++n) {
        double v = 0; for (size_t h = 0; h < amps.size(); ++h)
            v += amps[h] * std::sin(2 * std::numbers::pi * f0 * double(h + 1) * double(n) / fs);
        x[n] = static_cast<float>(0.5 * v);
    }
    return x;
}

TEST_CASE("T-027 YIN f0 on harmonic tones", "[T-027][unit]") {
    for (double f : {146.83, 185.0, 440.0, 880.0, 1864.66}) {
        const auto x = harmonics(48000, f, {1.0, 0.5, 0.3, 0.2});
        // YIN with parabolic interpolation resolves well under 1 cent at 1 s duration; allow 2 cents.
        CHECK(std::abs(cents(estimateF0(x, 48000), f)) < 2.0);
    }
    CHECK(estimateF0(std::vector<float>(48000, 0.0f), 48000) == 0.0);
}

TEST_CASE("T-027 register classification", "[T-027][unit]") {
    CHECK(classifyRegime(harmonics(48000, 185.0, {1, .1, .5}), 48000, 185.0) == Regime::FirstRegister);
    CHECK(classifyRegime(harmonics(48000, 555.0, {1, .5}), 48000, 185.0) == Regime::Twelfth);
    CHECK(classifyRegime(harmonics(48000, 370.0, {1, .5}), 48000, 185.0) == Regime::Other); // octave: not a clarinet register
    CHECK(classifyRegime(harmonics(48000, 925.0, {1, .5}), 48000, 185.0) == Regime::Other);
    CHECK(classifyRegime(std::vector<float>(48000, 0.0f), 48000, 185.0) == Regime::Silent);
}

TEST_CASE("T-027 harmonic levels, centroid and cents", "[T-027][unit]") {
    const auto x = harmonics(48000, 200.0, {1.0, 0.5, 0.25});
    const auto L = harmonicLevelsDb(x, 48000, 200.0, 3);
    REQUIRE(L.size() == 3);
    // Hann window, 1 s, bin width 1 Hz: amplitude ratios recovered within 0.1 dB.
    CHECK(L[0] == Catch::Approx(0.0).margin(0.1));
    CHECK(L[1] == Catch::Approx(-6.0206).margin(0.1));
    CHECK(L[2] == Catch::Approx(-12.0412).margin(0.1));
    // Power-weighted centroid of {200,400,600} with powers {1,.25,.0625}: 257.14 Hz; leakage < 1 %.
    CHECK(spectralCentroidHz(x, 48000) == Catch::Approx(257.14).epsilon(0.01));
    CHECK(cents(880.0, 440.0) == Catch::Approx(1200.0).epsilon(1e-12));
    CHECK_THROWS(harmonicLevelsDb(x, 48000, 0.0, 3));
}
