// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-015 (performance / real-time safety) — SC-7: no heap allocation on the audio thread (D-010).
#include "TestSupport.h"
#include <atomic>
#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include <new>
#include <vector>

static std::atomic<bool> g_track{false};
static std::atomic<long> g_allocs{0};

void* operator new(std::size_t n) {
    if (g_track.load(std::memory_order_relaxed)) g_allocs.fetch_add(1, std::memory_order_relaxed);
    if (void* p = std::malloc(n ? n : 1)) return p;
    throw std::bad_alloc();
}
void* operator new[](std::size_t n) { return operator new(n); }
void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }
void operator delete[](void* p, std::size_t) noexcept { std::free(p); }

TEST_CASE("T-015 process(), note and parameter calls never allocate", "[T-015][performance]") {
    clar::ClarinetVoice v(clartest::embeddedTable());
    v.prepare(48000, 512);
    std::vector<float> buf(512);
    g_allocs = 0; g_track = true;
    v.setBreath(-1);
    for (int i = 0; i < 400; ++i) {
        if (i % 40 == 0) v.noteOn(49 + (i / 40) % 30, 0.7f);
        if (i % 40 == 30) v.noteOff(49 + (i / 40) % 30);
        clar::VoiceParameters p; p.overblow = float(i % 10) / 10.0f; p.harmonicMode = (i / 100) % 2;
        v.setParameters(p); v.setPitchBend(0.1f); v.setAftertouch(0.3f);
        v.process(buf.data(), 512);
        (void)v.currentKeys();
    }
    g_track = false;
    CHECK(g_allocs.load() == 0);
}
