// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-019 (integration) — SC-2 lever highlighting, SC-6 mono legato; decisions D-007, D-010, D-011.
#include "TestSupport.h"
#include "clar/Fingering.h"
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <vector>
using namespace clar;

static double rmsOf(const std::vector<float>& x) {
    double s = 0; for (float v : x) s += double(v) * v; return x.empty() ? 0 : std::sqrt(s / double(x.size()));
}
static std::vector<float> run(ClarinetVoice& v, double seconds, double fs = 48000, int block = 256) {
    std::vector<float> out(static_cast<size_t>(seconds * fs));
    for (size_t i = 0; i < out.size(); i += static_cast<size_t>(block))
        v.process(out.data() + i, static_cast<int>(std::min<size_t>(static_cast<size_t>(block), out.size() - i)));
    return out;
}

TEST_CASE("T-019 out-of-range notes are ignored", "[T-019][integration]") {
    ClarinetVoice v(clartest::embeddedTable()); v.prepare(48000, 256); v.setBreath(-1);
    v.noteOn(40, 0.8f);
    v.noteOn(100, 0.8f);
    const auto x = run(v, 0.5);
    CHECK(v.currentConcertNote() == -1);
    CHECK(v.currentKeys().none());
    CHECK(rmsOf(x) < 1e-4);
}

TEST_CASE("T-019 levers follow the sounding note within one block; last-note priority legato", "[T-019][integration]") {
    ClarinetVoice v(clartest::embeddedTable()); v.prepare(48000, 256); v.setBreath(-1);
    v.noteOn(60, 0.7f);                                  // written D4
    run(v, 256.0 / 48000);                               // exactly one block
    CHECK(v.currentConcertNote() == 60);
    CHECK(v.currentKeys() == standardFingering(62).keys);
    run(v, 0.3);
    v.noteOn(72, 0.7f);                                  // written D5 (clarion)
    run(v, 256.0 / 48000);
    CHECK(v.currentConcertNote() == 72);
    CHECK(v.currentKeys() == standardFingering(74).keys);
    v.noteOff(72); run(v, 256.0 / 48000);
    CHECK(v.currentConcertNote() == 60);                 // falls back to the held note
    CHECK(v.currentKeys() == standardFingering(62).keys);
    v.noteOff(60);
    const auto tail = run(v, 1.0);
    CHECK(v.currentConcertNote() == -1);
    CHECK(v.currentKeys().none());
    CHECK(rmsOf(std::vector<float>(tail.end() - 9600, tail.end())) < 1e-4); // last 0.2 s silent
}

TEST_CASE("T-019 overblown-fingering mode shows the chalumeau fingering without register key", "[T-019][integration]") {
    ClarinetVoice v(clartest::embeddedTable()); v.prepare(48000, 256); v.setBreath(-1);
    VoiceParameters p; p.overblownFingering = true; v.setParameters(p);
    v.noteOn(72, 0.7f); run(v, 256.0 / 48000);
    CHECK(v.currentKeys() == standardFingering(74 - 19).keys);
    CHECK_FALSE(v.currentKeys().test(index(KeyId::Register)));
}

TEST_CASE("T-019 breath controller overrides velocity once used", "[T-019][integration]") {
    ClarinetVoice v(clartest::embeddedTable()); v.prepare(48000, 256);
    v.setBreath(0.0f);                                    // breath controller present, no air
    v.noteOn(60, 1.0f);
    const auto silent = run(v, 0.5);
    CHECK(rmsOf(silent) < 1e-3);
    v.setBreath(0.7f);
    const auto loud = run(v, 0.5);
    CHECK(rmsOf(std::vector<float>(loud.begin() + 12000, loud.end())) > 1e-2);
}
