// SPDX-License-Identifier: Apache-2.0
// STUB (S-000): replaced during implementation. Non-noexcept functions throw NotImplemented;
// noexcept functions return the sentinel documented in the header (plan/DECISIONS.md D-012).
#include "clar/ColinotReference.h"
#include "clar/Errors.h"
namespace clar {
ColinotRun simulateColinot(const ResonatorParams&, const ReedParams&, double, double, double, double, double) {
    throw NotImplemented("simulateColinot");
}
ResonatorParams colinotDSharpTable2() { throw NotImplemented("colinotDSharpTable2"); }
}
