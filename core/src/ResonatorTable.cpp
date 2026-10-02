// SPDX-License-Identifier: Apache-2.0
// Immutable modal resonator table loaded from JSON (schema "clarinet-vst/resonators@1", D-009).
// Every failure mode of untrusted input surfaces as ParseError (T-006).
#include "clar/ResonatorTable.h"
#include "clar/Errors.h"
#include <cmath>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

namespace clar {
namespace {

constexpr std::size_t kMaxBytes = 4u << 20;
constexpr std::size_t kMinModes = 2, kMaxModes = 24;
constexpr double kMinScale = 0.8, kMaxScale = 1.25;

[[noreturn]] void fail(const std::string& what) { throw ParseError("resonator table: " + what); }

double number(const nlohmann::json& obj, const char* key) {
    const auto it = obj.find(key);
    if (it == obj.end() || !it->is_number()) fail(std::string("missing or non-numeric \"") + key + "\"");
    const double v = it->get<double>();
    if (!std::isfinite(v)) fail(std::string("non-finite \"") + key + "\"");
    return v;
}

Vent ventFromString(const std::string& s) {
    if (s == "none") return Vent::None;
    if (s == "register") return Vent::Register;
    if (s == "altissimo") return Vent::Altissimo;
    fail("unknown vent \"" + s + "\"");
}

} // namespace

ResonatorTable ResonatorTable::fromJson(std::string_view json) {
    if (json.size() > kMaxBytes) fail("input larger than 4 MiB");
    try {
        const nlohmann::json doc = nlohmann::json::parse(json.begin(), json.end(), nullptr, /*allow_exceptions=*/false);
        if (doc.is_discarded() || !doc.is_object()) fail("not a JSON object");
        const auto schema = doc.find("schema");
        if (schema == doc.end() || !schema->is_string() || schema->get<std::string>() != "clarinet-vst/resonators@1")
            fail("wrong or missing schema tag");
        const auto entries = doc.find("entries");
        if (entries == doc.end() || !entries->is_array()) fail("missing entries array");

        ResonatorTable table;
        for (const auto& e : *entries) {
            if (!e.is_object()) fail("entry is not an object");
            const auto w = e.find("written");
            if (w == e.end() || !w->is_number_integer()) fail("entry lacks an integer \"written\"");
            const auto v = e.find("vent");
            if (v == e.end() || !v->is_string()) fail("entry lacks \"vent\"");
            Entry entry;
            entry.written = w->get<int>();
            entry.vent = ventFromString(v->get<std::string>());
            for (const auto& other : table.entries_)
                if (other.written == entry.written && other.vent == entry.vent) fail("duplicate (written, vent) entry");
            const auto ts = e.find("tuning_scale");
            if (ts != e.end()) {
                if (!ts->is_number()) fail("non-numeric tuning_scale");
                const double scale = ts->get<double>();
                if (!std::isfinite(scale) || scale < kMinScale || scale > kMaxScale) fail("tuning_scale outside [0.8, 1.25]");
                entry.params.tuningScale = scale;
            }
            const auto modes = e.find("modes");
            if (modes == e.end() || !modes->is_array()) fail("entry lacks \"modes\"");
            if (modes->size() < kMinModes || modes->size() > kMaxModes) fail("mode count outside [2, 24]");
            double lastIm = -INFINITY;
            for (const auto& m : *modes) {
                if (!m.is_object()) fail("mode is not an object");
                const double reS = number(m, "re_s"), imS = number(m, "im_s");
                const double reC = number(m, "re_c"), imC = number(m, "im_c");
                if (!(reS < 0.0)) fail("unstable pole (Re(s) >= 0)");
                if (!(imS > lastIm)) fail("modes not sorted by Im(s)");
                lastIm = imS;
                entry.params.modes.push_back({{reS, imS}, {reC, imC}});
            }
            table.entries_.push_back(std::move(entry));
        }
        return table;
    } catch (const ParseError&) {
        throw;
    } catch (const std::exception& ex) {
        throw ParseError(std::string("resonator table: ") + ex.what());
    }
}

const ResonatorParams& ResonatorTable::lookup(int writtenMidi, Vent vent) const {
    for (const auto& e : entries_)
        if (e.written == writtenMidi && e.vent == vent) return e.params;
    throw std::out_of_range("ResonatorTable::lookup: no entry for the requested (written, vent)");
}

std::size_t ResonatorTable::size() const noexcept { return entries_.size(); }

} // namespace clar
