// SPDX-License-Identifier: Apache-2.0
// clar_calibrate — per-entry tuning calibration of data/clarinet_resonators.json (plan/DECISIONS.md D-008).
// Usage: clar_calibrate --in data/clarinet_resonators.json --out data/clarinet_resonators.json
// For every (written, vent) entry the standard note is rendered with the real voice (velocity 0.6, Overblow 0,
// 48 kHz, 1 s), the pitch error in cents against 12-TET is measured and `tuning_scale` is iterated
//   scale <- scale * 2^(-cents/1200)      (at most 8 iterations, stop at |cents| <= 2, scale in [0.8, 1.25]).
// Only the numeric value after each entry's "tuning_scale" key is rewritten; all other bytes are preserved.
#include "clar/Analysis.h"
#include "clar/ClarinetVoice.h"
#include "clar/Fingering.h"
#include "clar/Pitch.h"
#include "clar/ResonatorTable.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace {

constexpr double kFs = 48000.0;
constexpr int kBlock = 256;
constexpr int kMaxIterations = 8;
constexpr double kToleranceCents = 2.0;
constexpr double kMinScale = 0.8, kMaxScale = 1.25;

std::string ventName(clar::Vent v) {
    switch (v) {
        case clar::Vent::None: return "none";
        case clar::Vent::Register: return "register";
        case clar::Vent::Altissimo: return "altissimo";
    }
    return "none";
}

/// Position of the numeric text after `"tuning_scale": ` of the entry (written, vent); npos if absent.
std::size_t scaleValuePos(const std::string& text, int written, clar::Vent vent) {
    const std::string key = "{\"written\": " + std::to_string(written) + ", \"vent\": \"" + ventName(vent) + "\", \"tuning_scale\": ";
    const auto p = text.find(key);
    return p == std::string::npos ? std::string::npos : p + key.size();
}

double readScale(const std::string& text, std::size_t pos) {
    return std::strtod(text.c_str() + pos, nullptr);
}

void writeScale(std::string& text, std::size_t pos, double scale) {
    std::size_t end = pos;
    while (end < text.size() && text[end] != ',' && text[end] != '}') ++end;
    char buf[40];
    std::snprintf(buf, sizeof buf, "%.7g", scale);
    std::string num = buf;
    if (num.find_first_of(".e") == std::string::npos) num += ".0";
    text.replace(pos, end - pos, num);
}

/// Pitch (Hz) of the entry's standard note, rendered with the real voice. 0 if silent / aperiodic.
double measureHz(const std::string& json, int written) {
    auto table = std::make_shared<const clar::ResonatorTable>(clar::ResonatorTable::fromJson(json));
    clar::ClarinetVoice voice(table);
    voice.prepare(kFs, kBlock);
    voice.setParameters(clar::VoiceParameters{});
    voice.setBreath(-1.0f);
    voice.noteOn(written - clar::kTransposition, 0.6f);
    std::vector<float> out(static_cast<std::size_t>(1.0 * kFs));
    for (std::size_t i = 0; i < out.size(); i += kBlock) {
        const int n = static_cast<int>(std::min<std::size_t>(kBlock, out.size() - i));
        voice.process(out.data() + i, n);
    }
    const std::size_t start = static_cast<std::size_t>(0.3 * kFs);
    return clar::estimateF0(std::span<const float>(out).subspan(start), kFs);
}

std::string slurp(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) { std::fprintf(stderr, "clar_calibrate: cannot read %s\n", path.c_str()); std::exit(1); }
    std::ostringstream ss; ss << in.rdbuf(); return ss.str();
}

} // namespace

int main(int argc, char** argv) {
    std::string in, out;
    for (int i = 1; i + 1 < argc; i += 2) {
        const std::string k = argv[i];
        if (k == "--in") in = argv[i + 1];
        else if (k == "--out") out = argv[i + 1];
        else { std::fprintf(stderr, "usage: clar_calibrate --in FILE --out FILE\n"); return 1; }
    }
    if (in.empty() || out.empty()) { std::fprintf(stderr, "usage: clar_calibrate --in FILE --out FILE\n"); return 1; }

    std::string text = slurp(in);
    int failed = 0;
    double worst = 0.0;
    for (int w = clar::kLowestWritten; w <= clar::kHighestWritten; ++w) {
        const clar::Vent vent = clar::standardFingering(w).vent;
        const std::size_t pos = scaleValuePos(text, w, vent);
        if (pos == std::string::npos) { std::fprintf(stderr, "entry %d/%s has no tuning_scale key\n", w, ventName(vent).c_str()); return 1; }
        const double target = clar::equalTemperedHz(w - clar::kTransposition);
        double scale = readScale(text, pos);
        double cents = 0.0;
        bool ok = false;
        int iter = 0;
        for (; iter <= kMaxIterations; ++iter) {
            const double hz = measureHz(text, w);
            if (hz <= 0.0) { cents = 9999.0; break; }
            cents = clar::cents(hz, target);
            if (std::fabs(cents) <= kToleranceCents) { ok = true; break; }
            if (std::fabs(cents) > 600.0 || iter == kMaxIterations) break; // wrong regime / not converging: report
            scale = std::clamp(scale * std::pow(2.0, -cents / 1200.0), kMinScale, kMaxScale);
            writeScale(text, pos, scale);
        }
        if (!ok) ++failed;
        worst = std::max(worst, std::fabs(cents) > 9000.0 ? 0.0 : std::fabs(cents));
        std::printf("written %2d %-9s scale %.5f  residual %+8.2f cents  iterations %d  %s\n",
                    w, ventName(vent).c_str(), scale, cents, iter, ok ? "ok" : "NOT CONVERGED");
    }
    std::ofstream o(out, std::ios::binary);
    o << text;
    std::printf("entries not converged: %d, worst residual %.2f cents\n", failed, worst);
    return failed == 0 ? 0 : 3;
}
