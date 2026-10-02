// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-003, T-004, T-005 (unit) — SC-2 highlighted levers; decisions D-004, D-007; claims C-010, C-025.
#include "TestSupport.h"
#include "clar/Fingering.h"
#include "clar/Pitch.h"
#include <catch2/catch_test_macros.hpp>
#include <stdexcept>
using namespace clar;

static KeySet keysFrom(const std::vector<std::string>& names) {
    KeySet s;
    for (const auto& n : names) { auto k = keyFromName(n); REQUIRE(k.has_value()); s.set(index(*k)); }
    return s;
}
static Vent ventFrom(const std::string& v) {
    if (v == "none") return Vent::None;
    if (v == "register") return Vent::Register;
    if (v == "altissimo") return Vent::Altissimo;
    FAIL("bad vent " << v); return Vent::None;
}

TEST_CASE("T-003 standard fingering chart matches the frozen fixture for all 45 notes", "[T-003][unit]") {
    const auto fx = clartest::loadFingeringFixture();
    REQUIRE(fx.size() == 45);
    for (const auto& f : fx) {
        INFO("written " << f.written);
        const Fingering& got = standardFingering(f.written);
        CHECK(got.writtenMidi == f.written);
        CHECK(got.keys == keysFrom(f.keys));
        CHECK(got.targetRegister == f.reg);
        CHECK(got.vent == ventFrom(f.vent));
    }
    CHECK_THROWS_AS(standardFingering(51), std::out_of_range);
    CHECK_THROWS_AS(standardFingering(97), std::out_of_range);
}

TEST_CASE("T-004 register key, thumb, vent and register-number invariants", "[T-004][unit]") {
    for (int w = kLowestWritten; w <= kHighestWritten; ++w) {
        INFO("written " << w);
        const auto& f = standardFingering(w);
        CHECK(f.keys.test(index(KeyId::Register)) == (w >= 70));      // throat Bb and above (C-010)
        CHECK(f.keys.test(index(KeyId::Thumb)) == (w <= 65 || w >= 71)); // thumb open for F#4..Bb4
        if (w <= 70)      { CHECK(f.targetRegister == 1); CHECK(f.vent == Vent::None); }
        else if (w <= 84) { CHECK(f.targetRegister == 2); CHECK(f.vent == Vent::Register); }
        else              { CHECK(f.targetRegister == 3); CHECK(f.vent == Vent::Altissimo); }
        if (w >= 71 && w <= 84) { // clarion = chalumeau fingering a twelfth below + register key (C-025)
            KeySet expected = standardFingering(w - 19).keys;
            expected.set(index(KeyId::Register));
            CHECK(f.keys == expected);
        }
    }
}

TEST_CASE("T-005 resolveNote: standard vs overblown-fingering mode", "[T-005][unit]") {
    for (int n = 0; n < 128; ++n) {
        INFO("concert " << n);
        const auto std_ = resolveNote(n, false);
        const auto ob   = resolveNote(n, true);
        const bool inRange = n >= 50 && n <= 94; // literal range, independent of isInRange()
        if (!inRange) { CHECK_FALSE(std_.has_value()); CHECK_FALSE(ob.has_value()); continue; }
        REQUIRE(std_.has_value()); REQUIRE(ob.has_value());
        const int w = n + 2;
        CHECK(std_->concertMidi == n);
        CHECK(std_->fingering == standardFingering(w));
        CHECK_FALSE(std_->overblownFingering);
        CHECK(std_->resonatorWritten == w);
        CHECK(std_->resonatorVent == standardFingering(w).vent);
        CHECK(ob->concertMidi == n);
        if (w >= 71 && w <= 84) {
            CHECK(ob->overblownFingering);
            CHECK(ob->fingering == standardFingering(w - 19));
            CHECK_FALSE(ob->fingering.keys.test(index(KeyId::Register)));
            CHECK(ob->resonatorWritten == w - 19);
            CHECK(ob->resonatorVent == Vent::None);
        } else {
            CHECK(*ob == *std_);
        }
    }
}
