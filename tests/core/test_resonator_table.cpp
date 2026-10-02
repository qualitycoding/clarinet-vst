// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-006 (unit, security: input validation) and T-026 (integration of the shipped table).
// Decisions D-009; claims C-006, C-007, C-013.
#include "TestSupport.h"
#include "clar/Errors.h"
#include "clar/Pitch.h"
#include "clar/ResonatorTable.h"
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <numbers>
#include <string>
using namespace clar;

TEST_CASE("T-006 parses the valid fixture", "[T-006][unit]") {
    const auto t = ResonatorTable::fromJson(clartest::readFile("resonators_valid.json"));
    CHECK(t.size() == 2);
    const auto& a = t.lookup(63, RegisterHole::None);
    REQUIRE(a.modes.size() == 4);
    CHECK(a.modes[0].pole == std::complex<double>(-17.59, 1195));
    CHECK(a.modes[1].residue == std::complex<double>(470.5, 0));
    CHECK(t.lookup(75, RegisterHole::Body).modes.size() == 8);
    CHECK_THROWS_AS(t.lookup(63, RegisterHole::Body), std::out_of_range);
    CHECK_THROWS_AS(t.lookup(99, RegisterHole::None), std::out_of_range);
}

TEST_CASE("T-006 rejects malformed or unsafe tables", "[T-006][unit][security]") {
    const std::string good = clartest::readFile("resonators_valid.json");
    auto mutate = [&](const std::string& from, const std::string& to) {
        std::string s = good; auto pos = s.find(from); REQUIRE(pos != std::string::npos);
        s.replace(pos, from.size(), to); return s;
    };
    CHECK_THROWS_AS(ResonatorTable::fromJson(""), ParseError);
    CHECK_THROWS_AS(ResonatorTable::fromJson("{"), ParseError);
    CHECK_THROWS_AS(ResonatorTable::fromJson("[]"), ParseError);
    CHECK_THROWS_AS(ResonatorTable::fromJson(mutate("resonators@1", "resonators@2")), ParseError);
    CHECK_THROWS_AS(ResonatorTable::fromJson(mutate("\"re_s\": -17.59", "\"re_s\": 17.59")), ParseError); // unstable
    CHECK_THROWS_AS(ResonatorTable::fromJson(mutate("\"im_s\": 2483", "\"im_s\": 1000")), ParseError);   // unsorted
    CHECK_THROWS_AS(ResonatorTable::fromJson(mutate("\"hole\": \"body\"", "\"hole\": \"bell\"")), ParseError);
    { // duplicate (written, hole) pair
        std::string d = mutate("\"written\": 75, \"hole\": \"body\"", "\"written\": 63, \"hole\": \"none\"");
        CHECK_THROWS_AS(ResonatorTable::fromJson(d), ParseError);
    }
    { // fewer than 4 modes
        std::string d = mutate(",\n      {\"re_s\": -269.34, \"im_s\": 4405, \"re_c\": 328.7, \"im_c\": 0}]}", "]}");
        CHECK_THROWS_AS(ResonatorTable::fromJson(d), ParseError);
    }
    CHECK_THROWS_AS(ResonatorTable::fromJson(mutate("\"im_s\": 1195", "\"im_s\": \"x\"")), ParseError);
}

TEST_CASE("T-026 shipped table covers every fingering with a physically plausible resonator", "[T-026][integration]") {
    const auto t = ResonatorTable::fromJson(embeddedResonatorJson());
    CHECK(t.size() == 33);
    for (int w = kLowestWritten; w <= kHighestWritten; ++w) {
        INFO("written " << w);
        const auto& f = standardFingering(w);
        const auto& r = t.lookup(w, f.hole);
        REQUIRE(r.modes.size() >= 4);
        REQUIRE(r.modes.size() <= 16);
        for (std::size_t i = 0; i < r.modes.size(); ++i) {
            CHECK(r.modes[i].pole.real() < 0.0);
            if (i) CHECK(r.modes[i].pole.imag() > r.modes[i - 1].pole.imag());
        }
        const double f1 = r.modes[0].pole.imag() / (2 * std::numbers::pi);
        const double target = equalTemperedHz(w - 9);
        if (f.hole == RegisterHole::None) {
            // First resonance within a quarter tone of the note (playing frequency sits slightly
            // below f1 because of reed compliance; C-013): -10..+50 cents.
            const double c = 1200 * std::log2(f1 / target);
            CHECK(c > -10.0); CHECK(c < 50.0);
            if (w <= 66) { // low fingerings: measured-like inharmonicity (C-007: ~2.065-2.08)
                const double ratio = r.modes[1].pole.imag() / r.modes[0].pole.imag();
                CHECK(ratio > 1.98); CHECK(ratio < 2.20);
            }
        }
    }
}
