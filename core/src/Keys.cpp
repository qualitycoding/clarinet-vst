// SPDX-License-Identifier: Apache-2.0
#include "clar/Keys.h"
#include <array>

namespace clar {
namespace {
constexpr std::array<std::string_view, kKeyCount> kNames{
    "register", "thumb", "a_key", "g_sharp", "lh1", "lh2", "lh_eb_sliver", "lh3", "lh_c_sharp", "lh_e",
    "lh_f", "lh_f_sharp", "side_1", "side_2", "side_3", "side_4", "rh1", "rh2", "rh_b_sliver", "rh3",
    "rh_e", "rh_f", "rh_f_sharp", "rh_g_sharp"};
} // namespace

std::string_view keyName(KeyId key) noexcept {
    const std::size_t i = index(key);
    return i < kKeyCount ? kNames[i] : std::string_view{};
}

std::optional<KeyId> keyFromName(std::string_view name) noexcept {
    for (std::size_t i = 0; i < kKeyCount; ++i)
        if (kNames[i] == name) return static_cast<KeyId>(i);
    return std::nullopt;
}

} // namespace clar
