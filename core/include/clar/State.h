// SPDX-License-Identifier: Apache-2.0
// Plugin state persistence (D-013). Format: UTF-8 JSON object, schema "clarinet-vst/state@1".
#pragma once
#include "clar/ClarinetVoice.h"
#include <optional>
#include <string>
#include <string_view>

namespace clar {

/// Throws NotImplemented until S-012.
std::string serializeState(const VoiceParameters& p);

/// Never throws. Returns nullopt for anything that is not a JSON object with the right schema
/// tag or is larger than 64 KiB; unknown keys ignored; missing keys -> defaults; values clamped().
/// Stub sentinel: std::nullopt.
std::optional<VoiceParameters> deserializeState(std::string_view text) noexcept;

} // namespace clar
