// SPDX-License-Identifier: Apache-2.0
// clar_render — offline renderer used by the realism comparison (D-015) and the G-003 bundle.
// Usage: clar_render --note N --velocity V --seconds S --fs F --out FILE.wav
//                   [--overblow O] [--overblown-fingering 0|1] [--release-at T]
// N is the concert MIDI note (50..94), V in [0,1], S in (0,120], F in [22050,192000], O in [0,1].
// Writes mono 32-bit float WAV. Exit codes: 0 ok, 1 bad arguments or runtime error, 2 not implemented.
#include "clar/ClarinetVoice.h"
#include "clar/Pitch.h"
#include "clar/ResonatorTable.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace {

bool parseDouble(const std::string& s, double& out) {
    char* end = nullptr;
    out = std::strtod(s.c_str(), &end);
    return end != s.c_str() && *end == '\0' && std::isfinite(out);
}

void put32(std::ofstream& f, std::uint32_t v) { for (int i = 0; i < 4; ++i) f.put(static_cast<char>((v >> (8 * i)) & 0xff)); }
void put16(std::ofstream& f, std::uint16_t v) { for (int i = 0; i < 2; ++i) f.put(static_cast<char>((v >> (8 * i)) & 0xff)); }

bool writeWav(const std::string& path, const std::vector<float>& x, std::uint32_t fs) {
    std::ofstream f(path, std::ios::binary);
    if (!f) return false;
    const std::uint32_t bytes = static_cast<std::uint32_t>(x.size() * sizeof(float));
    f.write("RIFF", 4); put32(f, 36 + bytes); f.write("WAVE", 4);
    f.write("fmt ", 4); put32(f, 16); put16(f, 3 /* IEEE float */); put16(f, 1 /* mono */);
    put32(f, fs); put32(f, fs * 4); put16(f, 4); put16(f, 32);
    f.write("data", 4); put32(f, bytes);
    f.write(reinterpret_cast<const char*>(x.data()), static_cast<std::streamsize>(bytes));
    return static_cast<bool>(f);
}

int usage() {
    std::fprintf(stderr,
        "usage: clar_render --note N --velocity V --seconds S --fs F --out FILE.wav\n"
        "                   [--overblow O] [--overblown-fingering 0|1] [--release-at T]\n");
    return 1;
}

} // namespace

int main(int argc, char** argv) {
    std::map<std::string, std::string> a;
    for (int i = 1; i < argc; i += 2) {
        if (i + 1 >= argc || std::strncmp(argv[i], "--", 2) != 0) return usage();
        a[argv[i]] = argv[i + 1];
    }
    for (const char* req : {"--note", "--velocity", "--seconds", "--fs", "--out"})
        if (!a.count(req)) return usage();
    for (const auto& kv : a) {
        static const char* const known[] = {"--note", "--velocity", "--seconds", "--fs", "--out", "--overblow", "--overblown-fingering", "--release-at"};
        if (std::find_if(std::begin(known), std::end(known), [&](const char* k) { return kv.first == k; }) == std::end(known)) return usage();
    }
    double note = 0, vel = 0, secs = 0, fs = 0, ob = 0, fingering = 0, release = -1.0;
    if (!parseDouble(a["--note"], note) || !parseDouble(a["--velocity"], vel) || !parseDouble(a["--seconds"], secs) ||
        !parseDouble(a["--fs"], fs)) return usage();
    if (a.count("--overblow") && !parseDouble(a["--overblow"], ob)) return usage();
    if (a.count("--overblown-fingering") && !parseDouble(a["--overblown-fingering"], fingering)) return usage();
    if (a.count("--release-at") && !parseDouble(a["--release-at"], release)) return usage();
    if (note != std::floor(note) || !clar::isInRange(static_cast<int>(note))) { std::fprintf(stderr, "note outside concert 50..94\n"); return 1; }
    if (vel < 0.0 || vel > 1.0 || ob < 0.0 || ob > 1.0 || secs <= 0.0 || secs > 120.0 || fs < 22050.0 || fs > 192000.0) { std::fprintf(stderr, "argument out of range\n"); return 1; }

    try {
        auto table = std::make_shared<const clar::ResonatorTable>(clar::ResonatorTable::fromJson(clar::embeddedResonatorJson()));
        clar::ClarinetVoice voice(table);
        constexpr int block = 256;
        voice.prepare(fs, block);
        clar::VoiceParameters p;
        p.overblow = static_cast<float>(ob);
        p.overblownFingering = fingering >= 0.5;
        voice.setParameters(p);
        voice.setBreath(-1.0f);
        voice.noteOn(static_cast<int>(note), static_cast<float>(vel));
        std::vector<float> out(static_cast<std::size_t>(std::llround(secs * fs)));
        const std::size_t releaseAt = release >= 0.0 ? static_cast<std::size_t>(std::llround(release * fs)) : out.size() + 1;
        bool released = false;
        for (std::size_t i = 0; i < out.size();) {
            if (!released && i >= releaseAt) { voice.noteOff(static_cast<int>(note)); released = true; }
            std::size_t n = std::min<std::size_t>(block, out.size() - i);
            if (!released && releaseAt > i) n = std::min(n, releaseAt - i);   // split the block exactly at the release
            voice.process(out.data() + i, static_cast<int>(n));
            i += n;
        }
        if (!writeWav(a["--out"], out, static_cast<std::uint32_t>(fs))) { std::fprintf(stderr, "cannot write %s\n", a["--out"].c_str()); return 1; }
    } catch (const std::exception& e) {
        std::fprintf(stderr, "clar_render: %s\n", e.what());
        return 1;
    }
    return 0;
}
