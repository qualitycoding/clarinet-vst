// SPDX-License-Identifier: Apache-2.0
// STUB (S-000): replaced during implementation. Non-noexcept functions throw NotImplemented;
// noexcept functions return the sentinel documented in the header (plan/DECISIONS.md D-012).
#include "clar/Fingering.h"
#include "clar/Errors.h"
namespace clar {
const Fingering& standardFingering(int) { throw NotImplemented("standardFingering"); }
std::optional<NoteResolution> resolveNote(int, bool) noexcept { return std::nullopt; }
}
