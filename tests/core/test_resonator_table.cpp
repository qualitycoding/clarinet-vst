// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-006 (unit, security: input validation) and T-026 (integration of the shipped table).
// Decisions D-008, D-009; claims C-006, C-013, C-027, C-029.
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
    const auto& a = t.lookup(55, Vent::None);
    REQUIRE(a.modes.size() == 4);
    CHECK(a.modes[0].pole == std::complex<double>(-18.2212, 1112.75));
    CHECK(a.modes[1].residue == std::complex<double>(700.486, 11.7645));
    CHECK(a.tuningScale == 1.0);
    const auto& b = t.lookup(74, Vent::Register);
    CHECK(b.modes.size() == 10);
    CHECK(b.tuningScale == 1.0);                                   // absent -> default 1.0
    CHECK_THROWS_AS(t.lookup(55, Vent::Register), std::out_of_range);
    CHECK_THROWS_AS(t.lookup(74, Vent::Altissimo), std::out_of_range);
    CHECK_THROWS_AS(t.lookup(99, Vent::None), std::out_of_range);
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
    CHECK_THROWS_AS(ResonatorTable::fromJson(mutate("\"re_s\": -18.2212", "\"re_s\": 18.2212")), ParseError);  // unstable
    CHECK_THROWS_AS(ResonatorTable::fromJson(mutate("\"im_s\": 3355.22", "\"im_s\": 1000")), ParseError);    // unsorted
    CHECK_THROWS_AS(ResonatorTable::fromJson(mutate("\"vent\": \"register\"", "\"vent\": \"bell\"")), ParseError);
    CHECK_THROWS_AS(ResonatorTable::fromJson(mutate("\"tuning_scale\": 1.0", "\"tuning_scale\": 1.5")), ParseError);
    CHECK_THROWS_AS(ResonatorTable::fromJson(mutate("\"im_s\": 1112.75", "\"im_s\": \"x\"")), ParseError);
    CHECK_THROWS_AS(ResonatorTable::fromJson(mutate("\"im_s\": 1112.75", "\"im_s\": 1e999")), ParseError); // non-finite
    { // duplicate (written, vent) pair
        std::string d = mutate("\"written\": 74, \"vent\": \"register\"", "\"written\": 55, \"vent\": \"none\"");
        CHECK_THROWS_AS(ResonatorTable::fromJson(d), ParseError);
    }
    { // fewer than 2 modes
        std::string d = mutate(",\n      {\"re_s\": -31.4159, \"im_s\": 3355.22, \"re_c\": 700.486, \"im_c\": 11.7645},\n"
                               "      {\"re_s\": -41.1549, \"im_s\": 5580.73, \"re_c\": 684.037, \"im_c\": 15.5449},\n"
                               "      {\"re_s\": -50.8938, \"im_s\": 7749.05, \"re_c\": 648.69, \"im_c\": 22.6403}]}", "]}");
        CHECK_THROWS_AS(ResonatorTable::fromJson(d), ParseError);
    }
    CHECK_THROWS_AS(ResonatorTable::fromJson(good + std::string(5u << 20, ' ')), ParseError); // > 4 MiB
}

TEST_CASE("T-026 shipped table covers every fingering with a plausible clarinet resonator", "[T-026][integration]") {
    const auto t = ResonatorTable::fromJson(embeddedResonatorJson());
    CHECK(t.size() == 45);
    for (int w = kLowestWritten; w <= kHighestWritten; ++w) {
        INFO("written " << w);
        const auto& f = standardFingering(w);
        const auto& r = t.lookup(w, f.vent);
        REQUIRE(r.modes.size() >= 2);
        REQUIRE(r.modes.size() <= 24);
        CHECK(r.tuningScale >= 0.8); CHECK(r.tuningScale <= 1.25);
        for (std::size_t i = 0; i < r.modes.size(); ++i) {
            CHECK(r.modes[i].pole.real() < 0.0);
            if (i) CHECK(r.modes[i].pole.imag() > r.modes[i - 1].pole.imag());
        }
        const double target = equalTemperedHz(w - 2);
        auto hz = [&](std::size_t i) { return r.tuningScale * r.modes[i].pole.imag() / (2 * std::numbers::pi); };
        if (f.vent == Vent::None) {
            // first resonance within a quarter tone above the note; the playing frequency sits below f1
            // because of reed compliance (C-013); calibration (D-008) may move it by up to ~100 cents.
            const double c = 1200 * std::log2(hz(0) / target);
            CHECK(c > -30.0); CHECK(c < 150.0);
            if (w <= 65) { // closed cylinder: odd-harmonic resonance series, f2/f1 near 3 (C-025)
                const double ratio = r.modes[1].pole.imag() / r.modes[0].pole.imag();
                CHECK(ratio > 2.85); CHECK(ratio < 3.15);
            }
        } else if (f.vent == Vent::Register) {
            // second resonance carries the note (twelfth), first resonance is detuned upward and
            // strongly damped by the D-009 emulation (bandwidth ratio >= 5 vs mode 2).
            const double c = 1200 * std::log2(hz(1) / target);
            CHECK(std::abs(c) < 150.0);
            CHECK(-r.modes[0].pole.real() >= 5.0 * -r.modes[1].pole.real());
        } else {
            const double c = 1200 * std::log2(hz(0) / target);
            CHECK(c > -30.0); CHECK(c < 200.0);
        }
    }
    // every closed-hole entry needed by the overblown-fingering mode exists (D-007)
    for (int w = 71; w <= 84; ++w) CHECK_NOTHROW(t.lookup(w - 19, Vent::None));
}
