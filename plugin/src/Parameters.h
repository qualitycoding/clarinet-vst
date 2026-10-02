// SPDX-License-Identifier: Apache-2.0
// Automatable parameter IDs (D-013). IDs are permanent: never rename (host automation depends on them).
#pragma once
namespace clarplug::param {
inline constexpr const char* overblow     = "overblow";
inline constexpr const char* overblownFingering = "overblown_fingering";
inline constexpr const char* reedHardness = "reed_hardness";
inline constexpr const char* brightness   = "brightness";
inline constexpr const char* breathNoise  = "breath_noise";
inline constexpr const char* vibratoRate  = "vibrato_rate";
inline constexpr const char* vibratoDepth = "vibrato_depth";
inline constexpr const char* portamento   = "portamento";
inline constexpr const char* tuning       = "tuning_a4";
inline constexpr const char* outputGain   = "output_gain";
inline constexpr int count = 10;
}
