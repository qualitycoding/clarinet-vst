// SPDX-License-Identifier: Apache-2.0
// The 24 player-operated keys, rings and tone holes of a Boehm-system B-flat clarinet that the UI can
// highlight ("levers", A-006). Names follow the Woodwind Fingering Guide key scheme (C-010, D-004).
// A finger hole with a ring (thumb, LH1-3, RH1-3) counts as one lever: pressed = hole covered.
#pragma once
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace clar {

enum class KeyId : std::uint8_t {
    Register,    ///< left-thumb register key (R)                    "register"
    Thumb,       ///< left-thumb tone hole with ring (T)              "thumb"
    AKey,        ///< LH1 top A key (A)                               "a_key"
    GSharp,      ///< LH1 side G# key (G#)                            "g_sharp"
    LH1,         ///< LH first-finger hole with ring (1)              "lh1"
    LH2,         ///< LH second-finger hole with ring (2)             "lh2"
    LHEbSliver,  ///< LH Eb/Bb sliver key (Eb)                        "lh_eb_sliver"
    LH3,         ///< LH third-finger hole (3)                        "lh3"
    LHCSharp,    ///< LH pinky C#/G# key (C#)                         "lh_c_sharp"
    LHE,         ///< LH pinky low E/B key (E)                        "lh_e"
    LHF,         ///< LH pinky low F/C key (F)                        "lh_f"
    LHFSharp,    ///< LH pinky low F#/C# key (F#)                     "lh_f_sharp"
    Side1,       ///< RH first (uppermost) side trill key (1)         "side_1"
    Side2,       ///< RH second side trill key (2)                    "side_2"
    Side3,       ///< RH third side trill key (3)                     "side_3"
    Side4,       ///< RH fourth side key, Eb/Bb (4)                   "side_4"
    RH1,         ///< RH first-finger hole with ring (1)              "rh1"
    RH2,         ///< RH second-finger hole with ring (2)             "rh2"
    RHBSliver,   ///< RH B/F# sliver key (B)                          "rh_b_sliver"
    RH3,         ///< RH third-finger hole with ring (3)              "rh3"
    RHE,         ///< RH pinky low E/B key (E)                        "rh_e"
    RHF,         ///< RH pinky low F/C key (F)                        "rh_f"
    RHFSharp,    ///< RH pinky low F#/C# key (F#)                     "rh_f_sharp"
    RHGSharp,    ///< RH pinky G#/D# key (G#)                         "rh_g_sharp"
    Count
};

inline constexpr std::size_t kKeyCount = static_cast<std::size_t>(KeyId::Count); // 24
using KeySet = std::bitset<kKeyCount>;

inline constexpr std::size_t index(KeyId k) noexcept { return static_cast<std::size_t>(k); }

/// Stable snake_case identifier shown in comments above (used in fixtures). Stub sentinel: "".
std::string_view keyName(KeyId key) noexcept;

/// Inverse of keyName. Stub sentinel: std::nullopt.
std::optional<KeyId> keyFromName(std::string_view name) noexcept;

} // namespace clar
