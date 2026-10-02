// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-014, T-021, T-025 (operational) — SC-9 robustness; decisions D-010, D-014.
#include "TestSupport.h"
#include "clar/Analysis.h"
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <random>
#include <stdexcept>
#include <vector>
using namespace clar;

static std::vector<float> chaos(unsigned seed, double seconds) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> u(0.0f, 1.0f);
    std::uniform_int_distribution<int> note(40, 90), blk(64, 512);
    const double fs = 48000;
    ClarinetVoice v(clartest::embeddedTable()); v.prepare(fs, 512);
    std::vector<float> out(static_cast<size_t>(seconds * fs));
    size_t i = 0, nextEvent = 0;
    while (i < out.size()) {
        if (i >= nextEvent) {
            VoiceParameters p;
            p.overblow = u(rng) * 1.4f - 0.2f;  p.harmonicMode = u(rng) > 0.5f;
            p.reedHardness = u(rng) * 1.4f - 0.2f; p.brightness = u(rng); p.breathNoise = u(rng);
            p.vibratoRateHz = u(rng) * 10; p.vibratoDepth = u(rng); p.portamentoMs = u(rng) * 600;
            p.tuningA4Hz = 400 + u(rng) * 80; p.outputGainDb = u(rng) * 40 - 30;
            if (u(rng) < 0.02f) p.overblow = std::nanf("");
            v.setParameters(p);
            if (u(rng) < 0.5f) v.noteOn(note(rng), u(rng)); else v.noteOff(note(rng));
            v.setPitchBend(u(rng) * 2 - 1); v.setAftertouch(u(rng));
            v.setBreath(u(rng) < 0.3f ? -1.0f : u(rng));
            nextEvent = i + static_cast<size_t>(fs * 0.05);
        }
        const int n = static_cast<int>(std::min<size_t>(static_cast<size_t>(blk(rng)), out.size() - i));
        v.process(out.data() + i, n);
        i += static_cast<size_t>(n);
    }
    return out;
}

TEST_CASE("T-014 random abuse never produces NaN/Inf or exceeds full scale", "[T-014][operational]") {
    const auto x = chaos(20261001u, 5.0);
    bool finite = true, bounded = true;
    for (float s : x) { finite &= std::isfinite(s); bounded &= std::abs(s) <= 1.0f; }
    CHECK(finite);
    CHECK(bounded);
}

TEST_CASE("T-021 rendering is deterministic", "[T-021][operational]") {
    CHECK(chaos(7u, 2.0) == chaos(7u, 2.0));
}

TEST_CASE("T-025 extreme but valid host configurations work", "[T-025][operational]") {
    for (double fs : {22050.0, 192000.0})
        for (int block : {1, 4096}) {
            INFO("fs " << fs << " block " << block);
            const auto x = clartest::renderNote(fs, 61, 0.7f, 0.5, {}, block);
            double s = 0; bool finite = true;
            for (float v : x) { s += double(v) * v; finite &= std::isfinite(v); }
            CHECK(finite);
            CHECK(std::sqrt(s / double(x.size())) > 1e-3);
        }
}

TEST_CASE("T-025 invalid configuration is rejected; reset silences; zero-length blocks are safe", "[T-025][operational]") {
    ClarinetVoice v(clartest::embeddedTable());
    CHECK_THROWS_AS(v.prepare(8000, 256), std::invalid_argument);
    CHECK_THROWS_AS(v.prepare(48000, 0), std::invalid_argument);
    v.prepare(48000, 256); v.setBreath(-1);
    v.process(nullptr, 0);
    v.noteOn(61, 0.8f);
    std::vector<float> buf(256);
    for (int i = 0; i < 100; ++i) v.process(buf.data(), 256);
    v.reset();
    v.process(buf.data(), 256);
    double s = 0; for (float x : buf) s += double(x) * x;
    CHECK(std::sqrt(s / 256) < 1e-3);
    CHECK(v.currentConcertNote() == -1);
}
