// FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "clar/ClarinetVoice.h"
#include "clar/Fingering.h"
#include "clar/Keys.h"
#include "clar/ResonatorTable.h"
#include <algorithm>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace clartest {

struct FixtureFingering { int written; int reg; std::string vent; std::vector<std::string> keys; };

inline std::vector<FixtureFingering> loadFingeringFixture() {
    std::ifstream in(std::string(CLAR_FIXTURE_DIR) + "/clarinet_standard_fingerings.txt");
    if (!in) throw std::runtime_error("missing fingering fixture");
    std::vector<FixtureFingering> out;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ss(line);
        FixtureFingering f; std::string colon;
        ss >> f.written >> f.reg >> f.vent >> colon;
        if (colon != ":") throw std::runtime_error("bad fixture line: " + line);
        std::string k; while (ss >> k) f.keys.push_back(k);
        out.push_back(f);
    }
    return out;
}

inline std::string readFile(const std::string& name) {
    std::ifstream in(std::string(CLAR_FIXTURE_DIR) + "/" + name);
    if (!in) throw std::runtime_error("missing fixture " + name);
    std::stringstream ss; ss << in.rdbuf(); return ss.str();
}

inline std::shared_ptr<const clar::ResonatorTable> embeddedTable() {
    return std::make_shared<const clar::ResonatorTable>(
        clar::ResonatorTable::fromJson(clar::embeddedResonatorJson()));
}

/// Renders `seconds` of a single held note; breath controller absent (velocity drives blowing).
inline std::vector<float> renderNote(double fs, int concertNote, float velocity, double seconds,
                                     const clar::VoiceParameters& p = {}, int block = 256) {
    clar::ClarinetVoice v(embeddedTable());
    v.prepare(fs, block);
    v.setParameters(p);
    v.setBreath(-1.0f);
    v.noteOn(concertNote, velocity);
    std::vector<float> out(static_cast<size_t>(seconds * fs));
    for (size_t i = 0; i < out.size(); i += static_cast<size_t>(block)) {
        const int n = static_cast<int>(std::min<size_t>(static_cast<size_t>(block), out.size() - i));
        v.process(out.data() + i, n);
    }
    return out;
}

/// The steady-state analysis window used by pitch/register tests: [0.3 s, end).
inline std::vector<float> steady(const std::vector<float>& x, double fs) {
    const size_t start = static_cast<size_t>(0.3 * fs);
    return start < x.size() ? std::vector<float>(x.begin() + static_cast<long>(start), x.end()) : std::vector<float>{};
}

} // namespace clartest
