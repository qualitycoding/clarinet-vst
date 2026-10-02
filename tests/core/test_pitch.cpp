// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-001 (unit) — enforces SC-2 range/transposition; claims C-024.
#include "clar/Pitch.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <stdexcept>
using namespace clar;

TEST_CASE("T-001 B-flat clarinet range is concert D3..Bb6 and written = concert + 2", "[T-001][unit]") {
    for (int n = 0; n < 128; ++n) {
        const bool expected = n >= 50 && n <= 94;
        CHECK(isInRange(n) == expected);
        if (expected) {
            CHECK(writtenFromConcert(n) == n + 2);
            CHECK(concertFromWritten(n + 2) == n);
        } else {
            CHECK_THROWS_AS(writtenFromConcert(n), std::out_of_range);
        }
    }
    CHECK_THROWS_AS(concertFromWritten(51), std::out_of_range);
    CHECK_THROWS_AS(concertFromWritten(97), std::out_of_range);
}

TEST_CASE("T-001 equal temperament", "[T-001][unit]") {
    // Tolerance 1e-9 relative: closed form, only rounding error.
    CHECK(equalTemperedHz(69) == Catch::Approx(440.0).epsilon(1e-9));
    CHECK(equalTemperedHz(50) == Catch::Approx(146.83238395870376).epsilon(1e-9));
    CHECK(equalTemperedHz(94, 442.0) == Catch::Approx(1872.4721).epsilon(1e-6));
}
