// SPDX-License-Identifier: Apache-2.0
// Plugin state persistence (plan/DECISIONS.md D-013). Format: UTF-8 JSON object, schema "clarinet-vst/state@1".
#include "clar/State.h"
#include <algorithm>
#include <nlohmann/json.hpp>

namespace clar {

namespace {

constexpr std::string_view kSchema = "clarinet-vst/state@1";
constexpr std::size_t kMaxStateBytes = 64u * 1024u;
constexpr double kFloatLimit = 1.0e30; // keeps the double -> float conversion defined for any parsed number

void readFloat(const nlohmann::json& obj, const char* key, float& target) {
    const auto it = obj.find(key);
    if (it == obj.end() || !it->is_number()) return;
    const double v = it->get<double>();
    if (v != v) return; // NaN cannot come from JSON text, but never trust a parser
    target = static_cast<float>(std::clamp(v, -kFloatLimit, kFloatLimit));
}

void readBool(const nlohmann::json& obj, const char* key, bool& target) {
    const auto it = obj.find(key);
    if (it != obj.end() && it->is_boolean()) target = it->get<bool>();
}

} // namespace

std::string serializeState(const VoiceParameters& in) {
    const VoiceParameters p = clamped(in);
    nlohmann::json j;
    j["schema"] = std::string(kSchema);
    j["overblow"] = p.overblow;
    j["overblownFingering"] = p.overblownFingering;
    j["reedHardness"] = p.reedHardness;
    j["brightness"] = p.brightness;
    j["breathNoise"] = p.breathNoise;
    j["vibratoRateHz"] = p.vibratoRateHz;
    j["vibratoDepth"] = p.vibratoDepth;
    j["portamentoMs"] = p.portamentoMs;
    j["tuningA4Hz"] = p.tuningA4Hz;
    j["outputGainDb"] = p.outputGainDb;
    return j.dump();
}

std::optional<VoiceParameters> deserializeState(std::string_view text) noexcept {
    try {
        if (text.empty() || text.size() > kMaxStateBytes) return std::nullopt;
        const nlohmann::json j = nlohmann::json::parse(text.begin(), text.end(), nullptr, false);
        if (j.is_discarded() || !j.is_object()) return std::nullopt;
        const auto schema = j.find("schema");
        if (schema == j.end() || !schema->is_string() || schema->get<std::string>() != kSchema) return std::nullopt;

        VoiceParameters p;
        readFloat(j, "overblow", p.overblow);
        readBool(j, "overblownFingering", p.overblownFingering);
        readFloat(j, "reedHardness", p.reedHardness);
        readFloat(j, "brightness", p.brightness);
        readFloat(j, "breathNoise", p.breathNoise);
        readFloat(j, "vibratoRateHz", p.vibratoRateHz);
        readFloat(j, "vibratoDepth", p.vibratoDepth);
        readFloat(j, "portamentoMs", p.portamentoMs);
        readFloat(j, "tuningA4Hz", p.tuningA4Hz);
        readFloat(j, "outputGainDb", p.outputGainDb);
        return clamped(p);
    } catch (...) {
        return std::nullopt; // includes std::bad_alloc: the host keeps its current parameters
    }
}

} // namespace clar
