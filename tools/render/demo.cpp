// SPDX-License-Identifier: Apache-2.0
// clar_demo — renders the listening demos of the G-003 bundle (plan/GATES.md) with the real voice.
// Usage: clar_demo OUTPUT_DIR      Writes 16-bit mono 44.1 kHz WAV files (one per demo).
#include "clar/ClarinetVoice.h"
#include "clar/ResonatorTable.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace {

constexpr double kFs = 44100.0;
constexpr int kBlock = 256;

class Session {
public:
    Session() : voice_(std::make_shared<const clar::ResonatorTable>(clar::ResonatorTable::fromJson(clar::embeddedResonatorJson()))) {
        voice_.prepare(kFs, kBlock);
        voice_.setBreath(-1.0f);
        voice_.setParameters(params_);
    }
    clar::VoiceParameters& params() { return params_; }
    clar::ClarinetVoice& voice() { return voice_; }
    /// Renders `seconds`, calling `each(timeInSegment)` before every block (parameter automation).
    template <class F>
    void run(double seconds, F each) {
        const std::size_t total = static_cast<std::size_t>(std::llround(seconds * kFs));
        std::vector<float> chunk(kBlock);
        for (std::size_t done = 0; done < total;) {
            const int n = static_cast<int>(std::min<std::size_t>(kBlock, total - done));
            each(static_cast<double>(done) / kFs);
            voice_.setParameters(params_);
            voice_.process(chunk.data(), n);
            out_.insert(out_.end(), chunk.begin(), chunk.begin() + n);
            done += static_cast<std::size_t>(n);
        }
    }
    void run(double seconds) { run(seconds, [](double) {}); }
    void note(int concert, float vel, double hold, double gap) {
        voice_.noteOn(concert, vel); run(hold); voice_.noteOff(concert); run(gap);
    }
    bool save(const std::string& path) const {
        std::ofstream f(path, std::ios::binary);
        if (!f) return false;
        auto p32 = [&](std::uint32_t v) { for (int i = 0; i < 4; ++i) f.put(static_cast<char>((v >> (8 * i)) & 0xff)); };
        auto p16 = [&](std::uint16_t v) { for (int i = 0; i < 2; ++i) f.put(static_cast<char>((v >> (8 * i)) & 0xff)); };
        const std::uint32_t bytes = static_cast<std::uint32_t>(out_.size() * 2);
        f.write("RIFF", 4); p32(36 + bytes); f.write("WAVE", 4); f.write("fmt ", 4); p32(16); p16(1); p16(1);
        p32(static_cast<std::uint32_t>(kFs)); p32(static_cast<std::uint32_t>(kFs) * 2); p16(2); p16(16);
        f.write("data", 4); p32(bytes);
        for (float v : out_) p16(static_cast<std::uint16_t>(static_cast<std::int16_t>(std::lround(std::clamp(v, -1.0f, 1.0f) * 0.9f * 32767.0f))));
        return static_cast<bool>(f);
    }
private:
    clar::VoiceParameters params_;
    clar::ClarinetVoice voice_;
    std::vector<float> out_;
};

} // namespace

int main(int argc, char** argv) {
    if (argc != 2) { std::fprintf(stderr, "usage: clar_demo OUTPUT_DIR\n"); return 1; }
    const std::string dir = argv[1];
    int failures = 0;
    auto save = [&](Session& s, const char* name) { failures += !s.save(dir + "/" + name); };

    try {
        { // 1. chromatic scale over the whole range at mf
            Session s;
            for (int n = 50; n <= 94; ++n) s.note(n, 0.6f, 0.45, 0.12);
            save(s, "01-chromatic-scale-D3-Bb6.wav");
        }
        { // 2. legato phrase across the break (written G4 A4 Bb4 B4 C5 = concert 65 67 68 69 70) with vibrato
            Session s;
            s.params().vibratoDepth = 0.6f; s.params().vibratoRateHz = 5.5f; s.voice().setAftertouch(0.7f);
            const int seq[] = {65, 67, 68, 69, 70, 69, 68, 67, 65};
            s.voice().noteOn(seq[0], 0.65f); s.run(0.7);
            for (int i = 1; i < 9; ++i) { s.voice().noteOn(seq[i], 0.65f); s.run(0.6); s.voice().noteOff(seq[i - 1]); }
            s.voice().noteOff(seq[8]); s.run(0.6);
            save(s, "02-legato-across-the-break.wav");
        }
        { // 3. Overblow sweep 0 -> 1 on concert F3, 8 s
            Session s;
            s.voice().noteOn(53, 0.7f);
            s.run(8.0, [&](double t) { s.params().overblow = static_cast<float>(std::min(1.0, t / 7.0)); });
            s.voice().noteOff(53); s.run(0.5);
            save(s, "03-overblow-sweep-F3.wav");
        }
        { // 4. Overblown fingering: clarion notes played without the register key (concert 69..82)
            Session s;
            s.params().overblownFingering = true;
            for (int n = 69; n <= 82; ++n) s.note(n, 0.7f, 0.5, 0.12);
            save(s, "04-overblown-fingering-run.wav");
        }
        { // 5. altissimo scale (concert B5 .. Bb6)
            Session s;
            for (int n = 83; n <= 94; ++n) s.note(n, 0.6f, 0.5, 0.12);
            save(s, "05-altissimo-scale.wav");
        }
        { // 6. dynamics: pp mf ff on concert G4 (written A4)
            Session s;
            for (float v : {0.25f, 0.6f, 0.95f}) s.note(67, v, 1.6, 0.4);
            save(s, "06-dynamics-pp-mf-ff-G4.wav");
        }
    } catch (const std::exception& e) {
        std::fprintf(stderr, "clar_demo: %s\n", e.what());
        return 1;
    }
    std::printf("demos written to %s, failures %d\n", dir.c_str(), failures);
    return failures == 0 ? 0 : 1;
}
