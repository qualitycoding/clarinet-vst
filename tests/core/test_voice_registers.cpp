// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-011, T-012, T-013 (integration) — SC-5 registers and overblowing; decisions D-006, D-007, D-009;
// claims C-025, C-027, C-028; spikes C4, C5, C6, C8.
#include "TestSupport.h"
#include "clar/Analysis.h"
#include "clar/Pitch.h"
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <utility>
#include <vector>
using namespace clar;

TEST_CASE("T-011 with Overblow at 0, chalumeau fingerings speak in the first register", "[T-011][integration]") {
    for (int n = 50; n <= 63; ++n) // written E3..F4
        for (float v : {0.3f, 0.6f, 0.9f}) {
            INFO("concert " << n << " velocity " << v);
            const auto x = clartest::steady(clartest::renderNote(48000, n, v, 1.0), 48000);
            CHECK(classifyRegime(x, 48000, equalTemperedHz(n)) == Regime::FirstRegister);
        }
}

TEST_CASE("T-012 clarion notes speak as the twelfth of their chalumeau fingering", "[T-012][integration]") {
    for (int n = 69; n <= 82; ++n) // written B4..C6, register key
        for (float v : {0.3f, 0.6f, 0.9f}) {
            INFO("concert " << n << " velocity " << v);
            const auto x = clartest::steady(clartest::renderNote(48000, n, v, 1.0), 48000);
            CHECK(classifyRegime(x, 48000, equalTemperedHz(n - 19)) == Regime::Twelfth);
        }
}

TEST_CASE("T-012 overblown-fingering mode sounds the twelfth without the register key", "[T-012][integration]") {
    VoiceParameters p; p.overblownFingering = true;
    for (int n = 69; n <= 82; ++n) {
        INFO("concert " << n);
        const auto x = clartest::steady(clartest::renderNote(48000, n, 0.7f, 1.0, p), 48000);
        CHECK(classifyRegime(x, 48000, equalTemperedHz(n - 19)) == Regime::Twelfth);
        // +-50 cents: an un-vented twelfth is not corrected by a register hole (spike C6: -2..+4 cents
        // before calibration; real instruments: tens of cents, C-029).
        CHECK(std::abs(cents(estimateF0(x, 48000), equalTemperedHz(n))) <= 50.0);
    }
}

TEST_CASE("T-013 Overblow brightens the tone before the break", "[T-013][integration]") {
    for (int n : {52, 57, 62}) {
        INFO("concert " << n);
        double c[3]; int i = 0;
        for (float o : {0.0f, 0.25f, 0.5f}) {
            VoiceParameters p; p.overblow = o;
            const auto x = clartest::steady(clartest::renderNote(48000, n, 0.7f, 1.0, p), 48000);
            CHECK(classifyRegime(x, 48000, equalTemperedHz(n)) == Regime::FirstRegister);
            c[i++] = spectralCentroidHz(x, 48000);
        }
        CHECK(c[0] > 0.0);
        CHECK(c[1] > c[0]);
        CHECK(c[2] > c[1]);
    }
}

TEST_CASE("T-013 full Overblow breaks chalumeau notes away from the fundamental", "[T-013][integration]") {
    int broken = 0, total = 0;
    VoiceParameters p; p.overblow = 1.0f;
    for (int n = 50; n <= 63; ++n) {
        const auto x = clartest::steady(clartest::renderNote(48000, n, 0.8f, 1.0, p), 48000);
        const Regime r = classifyRegime(x, 48000, equalTemperedHz(n));
        broken += (r == Regime::Twelfth || r == Regime::Other); // twelfth or squeak, still sounding
        ++total;
    }
    INFO(broken << "/" << total);
    CHECK(broken >= (total * 8) / 10);
}

TEST_CASE("T-012 legato across the break reaches the clarion twelfth", "[T-012][integration]") {
    // Register changes while sounding are basin-dependent on real and simulated clarinets (C-004);
    // the voice must still land in the twelfth within 150 ms for normal playing (decision rule
    // "register-change articulation" in plan/DECISIONS.md).
    for (auto [from, to] : {std::pair{55, 69}, std::pair{60, 72}, std::pair{62, 79}}) {
        INFO("concert " << from << " -> " << to);
        ClarinetVoice v(clartest::embeddedTable()); v.prepare(48000, 256); v.setBreath(-1);
        v.noteOn(from, 0.6f);
        std::vector<float> buf(48000);
        for (int i = 0; i < 48000; i += 256) v.process(buf.data() + i, std::min(256, 48000 - i));
        v.noteOn(to, 0.6f);
        for (int i = 0; i < 48000; i += 256) v.process(buf.data() + i, std::min(256, 48000 - i));
        // analyse 150 ms .. 1 s after the change
        std::vector<float> tail(buf.begin() + 7200, buf.end());
        CHECK(classifyRegime(tail, 48000, equalTemperedHz(to - 19)) == Regime::Twelfth);
    }
}
