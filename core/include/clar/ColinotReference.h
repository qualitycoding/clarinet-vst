// SPDX-License-Identifier: Apache-2.0
// Offline (non-real-time) reference integrator used only for code verification (T-008 Colinot 2021 code verification; T-009 clarinet register-hole emulation).
#pragma once
#include "clar/ReedModel.h"
#include "clar/ResonatorTable.h"
#include <vector>

namespace clar {

struct ColinotRun {
    std::vector<float> pressure; ///< dimensionless mouthpiece pressure p(t)
    double sampleRate = 0.0;
};

/// Integrates eqs. (4),(7),(12),(13) with gamma(t) from eq. (17):
/// gamma(t) = gammaFinal/2 * (1 + tanh((t - 5 tauG)/tauG)).
/// Scheme (D-005): exact exponential modal update with flow held over the step; reed by
/// central differences; sampleRate default 176400 Hz as in the paper.
/// Throws std::invalid_argument for seconds <= 0, tauG <= 0, sampleRate < 44100, empty modes.
ColinotRun simulateColinot(const ResonatorParams& resonator, const ReedParams& reed,
                           double zeta, double gammaFinal, double tauG, double seconds,
                           double sampleRate = 176400.0);

/// The D# fingering of Colinot et al. 2021, Table 2 (8 modes), for tests.
ResonatorParams colinotDSharpTable2();

} // namespace clar
