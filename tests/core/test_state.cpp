// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-017 (unit + security fuzz) — SC-8 parameter persistence, SC-9 robustness; decision D-013.
#include "clar/State.h"
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <random>
#include <string>
using namespace clar;

TEST_CASE("T-017 state round-trips every parameter", "[T-017][unit]") {
    VoiceParameters p;
    p.overblow = 0.7f; p.harmonicMode = true; p.reedHardness = 0.2f; p.brightness = 0.9f;
    p.breathNoise = 0.1f; p.vibratoRateHz = 6.5f; p.vibratoDepth = 0.4f; p.portamentoMs = 120.0f;
    p.tuningA4Hz = 442.0f; p.outputGainDb = -6.0f;
    const auto text = serializeState(p);
    CHECK(text.find("clarinet-vst/state@1") != std::string::npos);
    const auto back = deserializeState(text);
    REQUIRE(back.has_value());
    CHECK(*back == p);
}

TEST_CASE("T-017 malformed state is rejected or clamped, never crashes", "[T-017][unit][security]") {
    CHECK_FALSE(deserializeState("").has_value());
    CHECK_FALSE(deserializeState("null").has_value());
    CHECK_FALSE(deserializeState("[1,2]").has_value());
    CHECK_FALSE(deserializeState("{\"schema\":\"other\"}").has_value());
    CHECK_FALSE(deserializeState(std::string(70000, ' ')).has_value()); // > 64 KiB
    const auto c = deserializeState("{\"schema\":\"clarinet-vst/state@1\",\"overblow\":99,\"tuningA4Hz\":1e9,\"unknown\":1}");
    REQUIRE(c.has_value());
    CHECK(c->overblow == 1.0f);
    CHECK(c->tuningA4Hz == 466.0f);
    const auto d = deserializeState("{\"schema\":\"clarinet-vst/state@1\"}");
    REQUIRE(d.has_value());
    CHECK(*d == VoiceParameters{});

    std::mt19937 rng(20261001u); // fixed seed
    std::uniform_int_distribution<int> len(0, 400), ch(0, 255);
    const std::string seed = serializeState(VoiceParameters{});
    for (int i = 0; i < 10000; ++i) {
        std::string s = (i % 2) ? seed : std::string();
        const int n = len(rng);
        for (int j = 0; j < n; ++j) {
            if (!s.empty() && (j % 3 == 0)) s[static_cast<size_t>(ch(rng)) % s.size()] = static_cast<char>(ch(rng));
            else s.push_back(static_cast<char>(ch(rng)));
        }
        const auto r = deserializeState(s);
        if (r) { // any accepted value must be within range
            CHECK(r->overblow >= 0.0f); CHECK(r->overblow <= 1.0f);
            CHECK(std::isfinite(r->tuningA4Hz));
        }
    }
}
