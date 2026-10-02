// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-010, T-020 (integration) — SC-3 intonation; decision D-008 (per-fingering tuning).
// Tolerance +-10 cents: below the ~10-20 cent intonation spread of real saxophones/players
// (Szwarcberg 2025 reports register inharmonicity of tens of cents), so a pass means "in tune".
#include "TestSupport.h"
#include "clar/Analysis.h"
#include "clar/Pitch.h"
#include <catch2/catch_test_macros.hpp>
#include <cmath>
using namespace clar;

TEST_CASE("T-010 every note sounds within 10 cents of equal temperament", "[T-010][integration]") {
    for (int n = kLowestConcert; n <= kHighestConcert; ++n) {
        INFO("concert " << n);
        const auto x = clartest::steady(clartest::renderNote(48000, n, 0.6f, 1.0), 48000);
        const double f0 = estimateF0(x, 48000);
        CHECK(std::abs(cents(f0, equalTemperedHz(n))) <= 10.0);
    }
}

TEST_CASE("T-010 tuning reference follows the A4 parameter", "[T-010][integration]") {
    VoiceParameters p; p.tuningA4Hz = 442.0f;
    const auto x = clartest::steady(clartest::renderNote(48000, 69, 0.6f, 1.0, p), 48000);
    CHECK(std::abs(cents(estimateF0(x, 48000), 442.0)) <= 10.0);
}

TEST_CASE("T-020 pitch is sample-rate independent", "[T-020][integration]") {
    for (int n : {49, 61, 73, 81}) {
        INFO("concert " << n);
        double f[3]; int i = 0;
        for (double fs : {44100.0, 48000.0, 96000.0})
            f[i++] = estimateF0(clartest::steady(clartest::renderNote(fs, n, 0.6f, 1.0), fs), fs);
        // 3 cents: inaudible, and well above YIN resolution (T-027: < 2 cents).
        CHECK(std::abs(cents(f[0], f[1])) < 3.0);
        CHECK(std::abs(cents(f[1], f[2])) < 3.0);
    }
}
