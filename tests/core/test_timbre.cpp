// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
// T-032 (integration, realism) — SC-4 clarinet fingerprint: below the tone-hole-lattice cutoff the
// even harmonics of low-register notes are much weaker than the odd ones (Petersen et al. 2020: "several
// decades weaker"; Benade & Kouzoupis 1988), claims C-025, C-030. Spike C2: H2 ~ -46 dB vs H3 ~ -10 dB in the
// internal pressure. Threshold 10 dB on the radiated output leaves room for the D-014 output stage.
#include "TestSupport.h"
#include "clar/Analysis.h"
#include "clar/Pitch.h"
#include <catch2/catch_test_macros.hpp>
using namespace clar;

TEST_CASE("T-032 low-register notes have odd-dominated spectra", "[T-032][integration]") {
    int ok = 0, total = 0;
    for (int n = 50; n <= 59; ++n) { // written E3..C#4: harmonics 2 and 3 lie below 1.5 kHz
        const auto x = clartest::steady(clartest::renderNote(48000, n, 0.6f, 1.0), 48000);
        const auto L = harmonicLevelsDb(x, 48000, equalTemperedHz(n), 3);
        ok += (L[2] - L[1]) >= 10.0;
        ++total;
    }
    INFO(ok << "/" << total);
    CHECK(ok >= (total * 8) / 10);
}
