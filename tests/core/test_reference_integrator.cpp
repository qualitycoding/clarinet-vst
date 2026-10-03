// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-008 (integration, code verification against a published model) — claims C-006, C-014.
// T-009 (integration) — register-hole nonlinear-loss emulation (D-009) makes the clarion speak;
// claims C-027, C-028; spikes research/spikes/reed_map.py (C4) and t009_check.py (C9).
#include "TestSupport.h"
#include "clar/Analysis.h"
#include "clar/ColinotReference.h"
#include <catch2/catch_test_macros.hpp>
#include <span>
#include <string>
using namespace clar;

// ---- T-008: Colinot, Vergez, Guillemain, Doc, Acta Acustica 5 (2021) 33, Tables 1-2, eq. (17).
// The paper reports F#3 (185 Hz) for this (saxophone) fingering; the D-005 scheme gives 189.1 Hz
// (spike S1 of the sibling saxophone plan, reproduced here as the integrator is instrument-independent).
// The +-4 % window covers scheme-dependent playing-frequency differences, not a tuned value.
static Regime regimeR1(const ColinotRun& r, double expected) {
    std::span<const float> s(r.pressure);
    return classifyRegime(s.subspan(s.size() / 2), r.sampleRate, expected);
}

TEST_CASE("T-008 reference integrator reproduces the published first register", "[T-008][integration]") {
    const auto run = simulateColinot(colinotDSharpTable2(), ReedParams{}, 0.6, 0.5, 0.010, 0.6);
    REQUIRE(run.sampleRate == 176400.0);
    std::span<const float> s(run.pressure);
    const double f0 = estimateF0(s.subspan(s.size() / 2), run.sampleRate);
    CHECK(f0 > 180.0); CHECK(f0 < 195.0);
    CHECK(regimeR1(run, 187.5) == Regime::FirstRegister);
}

TEST_CASE("T-008 invalid arguments are rejected", "[T-008][integration]") {
    const auto r = colinotDSharpTable2();
    CHECK_THROWS_AS(simulateColinot(r, {}, 0.6, 0.5, 0.01, 0.0), std::invalid_argument);
    CHECK_THROWS_AS(simulateColinot(r, {}, 0.6, 0.5, 0.0, 0.5), std::invalid_argument);
    CHECK_THROWS_AS(simulateColinot(r, {}, 0.6, 0.5, 0.01, 0.5, 8000.0), std::invalid_argument);
    CHECK_THROWS_AS(simulateColinot(ResonatorParams{}, {}, 0.6, 0.5, 0.01, 0.5), std::invalid_argument);
}

// ---- T-009: clarion fingering (written D5 = written G3 geometry with the register hole open) from the
// frozen fixture. Literature (Szwarcberg et al. 2024/2026, C-027): a linear modal model does not reliably
// produce the second register; spike C9 replaying this exact procedure: linear 3/6, emulated 6/6 twelfths.
// Thresholds keep a margin of one outcome on each side.
TEST_CASE("T-009 register-hole loss emulation makes the clarion speak the twelfth", "[T-009][integration]") {
    const auto table = ResonatorTable::fromJson(clartest::readFile("resonators_valid.json"));
    const ResonatorParams linear = table.lookup(74, Vent::Register);
    ResonatorParams emulated = linear;
    emulated.modes[0].pole = {10.0 * linear.modes[0].pole.real(), linear.modes[0].pole.imag()}; // D-009: x10 bandwidth
    ReedParams reed = clarinetReed();
    reed.qR = 0.7;                                                                              // D-011 lip damping
    const double closedNoteHz = 174.61;                                                         // written G3, concert F3
    int twelfthLinear = 0, twelfthEmulated = 0;
    for (double g : {0.5, 0.65})
        for (double tau : {3e-3, 1e-2, 3e-2}) {
            twelfthLinear   += regimeR1(simulateColinot(linear,   reed, 0.4, g, tau, 0.4), closedNoteHz) == Regime::Twelfth;
            twelfthEmulated += regimeR1(simulateColinot(emulated, reed, 0.4, g, tau, 0.4), closedNoteHz) == Regime::Twelfth;
        }
    INFO("linear " << twelfthLinear << "/6, emulated " << twelfthEmulated << "/6");
    CHECK(twelfthEmulated >= 5);
    CHECK(twelfthEmulated >= twelfthLinear + 2);
}
