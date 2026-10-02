// SPDX-License-Identifier: Apache-2.0
// STUB (S-000): replaced during implementation. Non-noexcept functions throw NotImplemented;
// noexcept functions return the sentinel documented in the header (plan/DECISIONS.md D-012).
#include "clar/State.h"
#include "clar/Errors.h"
namespace clar {
std::string serializeState(const VoiceParameters&) { throw NotImplemented("serializeState"); }
std::optional<VoiceParameters> deserializeState(std::string_view) noexcept { return std::nullopt; }
}
