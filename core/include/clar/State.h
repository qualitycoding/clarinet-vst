// SPDX-License-Identifier: Apache-2.0
// Plugin state persistence (D-013). Format: UTF-8 JSON object, schema "clarinet-vst/state@1".
#pragma once
#include "clar/ClarinetVoice.h"
#include <optional>
#include <string>
#include <string_view>

namespace clar {

/// Always succeeds (values are clamped() first); output is a single-line UTF-8 JSON object.
std::string serializeState(const VoiceParameters& p);

/// Never throws. Returns nullopt for anything that is not a JSON object with the right schema
/// tag or is larger than 64 KiB; unknown keys ignored; missing keys -> defaults; values clamped().

std::optional<VoiceParameters> deserializeState(std::string_view text) noexcept;

} // namespace clar
