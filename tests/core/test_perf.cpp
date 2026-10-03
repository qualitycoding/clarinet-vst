// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-016 (performance) — SC-7: real-time factor <= 0.05 (i.e. < 5 % of one core) at 48 kHz/128.
// Harness: median of 5 renders of 10 s (variance tolerance: median rejects outliers). Release only.
#include "TestSupport.h"
#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <vector>

TEST_CASE("T-016 real-time factor", "[T-016][performance]") {
#ifndef NDEBUG
    SKIP("T-016 must run in a Release build (see HANDOFF.md)");
#endif
    std::vector<double> rtf;
    for (int r = 0; r < 5; ++r) {
        clar::ClarinetVoice v(clartest::embeddedTable());
        v.prepare(48000, 128); v.setBreath(-1);
        clar::VoiceParameters p; p.overblow = 0.5f; p.vibratoDepth = 0.3f; v.setParameters(p);
        std::vector<float> buf(128);
        const auto t0 = std::chrono::steady_clock::now();
        for (int b = 0; b < 48000 * 10 / 128; ++b) {
            if (b % 375 == 0) v.noteOn(50 + (b / 375) % 45, 0.7f);
            v.process(buf.data(), 128);
        }
        const double secs = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
        rtf.push_back(secs / 10.0);
    }
    std::sort(rtf.begin(), rtf.end());
    INFO("median RTF " << rtf[2]);
    CHECK(rtf[2] <= 0.05);
}
