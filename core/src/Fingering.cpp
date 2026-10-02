// SPDX-License-Identifier: Apache-2.0
// Standard Boehm B-flat clarinet fingerings, written MIDI 52..96 (plan/DECISIONS.md D-004, C-010).
// Table transcribed from tests/fixtures/clarinet_standard_fingerings.txt; the fixture is authoritative (T-003).
#include "clar/Fingering.h"
#include "clar/Pitch.h"
#include <array>
#include <stdexcept>

namespace clar {
namespace {

constexpr unsigned long long bit(KeyId k) noexcept { return 1ULL << static_cast<unsigned>(k); }

struct Row {
    int written;
    int reg;
    Vent vent;
    unsigned long long mask;
};

constexpr std::array<Row, 45> kRows{{
    {52, 1, Vent::None, bit(KeyId::Thumb) | bit(KeyId::LH1) | bit(KeyId::LH2) | bit(KeyId::LH3) | bit(KeyId::LHE) | bit(KeyId::RH1) | bit(KeyId::RH2) | bit(KeyId::RH3)},
    {53, 1, Vent::None, bit(KeyId::Thumb) | bit(KeyId::LH1) | bit(KeyId::LH2) | bit(KeyId::LH3) | bit(KeyId::RH1) | bit(KeyId::RH2) | bit(KeyId::RH3) | bit(KeyId::RHF)},
    {54, 1, Vent::None, bit(KeyId::Thumb) | bit(KeyId::LH1) | bit(KeyId::LH2) | bit(KeyId::LH3) | bit(KeyId::LHFSharp) | bit(KeyId::RH1) | bit(KeyId::RH2) | bit(KeyId::RH3)},
    {55, 1, Vent::None, bit(KeyId::Thumb) | bit(KeyId::LH1) | bit(KeyId::LH2) | bit(KeyId::LH3) | bit(KeyId::RH1) | bit(KeyId::RH2) | bit(KeyId::RH3)},
    {56, 1, Vent::None, bit(KeyId::Thumb) | bit(KeyId::LH1) | bit(KeyId::LH2) | bit(KeyId::LH3) | bit(KeyId::RH1) | bit(KeyId::RH2) | bit(KeyId::RH3) | bit(KeyId::RHGSharp)},
    {57, 1, Vent::None, bit(KeyId::Thumb) | bit(KeyId::LH1) | bit(KeyId::LH2) | bit(KeyId::LH3) | bit(KeyId::RH1) | bit(KeyId::RH2)},
    {58, 1, Vent::None, bit(KeyId::Thumb) | bit(KeyId::LH1) | bit(KeyId::LH2) | bit(KeyId::LH3) | bit(KeyId::RH1)},
    {59, 1, Vent::None, bit(KeyId::Thumb) | bit(KeyId::LH1) | bit(KeyId::LH2) | bit(KeyId::LH3) | bit(KeyId::RH2)},
    {60, 1, Vent::None, bit(KeyId::Thumb) | bit(KeyId::LH1) | bit(KeyId::LH2) | bit(KeyId::LH3)},
    {61, 1, Vent::None, bit(KeyId::Thumb) | bit(KeyId::LH1) | bit(KeyId::LH2) | bit(KeyId::LH3) | bit(KeyId::LHCSharp)},
    {62, 1, Vent::None, bit(KeyId::Thumb) | bit(KeyId::LH1) | bit(KeyId::LH2)},
    {63, 1, Vent::None, bit(KeyId::Thumb) | bit(KeyId::LH1) | bit(KeyId::LH2) | bit(KeyId::Side4)},
    {64, 1, Vent::None, bit(KeyId::Thumb) | bit(KeyId::LH1)},
    {65, 1, Vent::None, bit(KeyId::Thumb)},
    {66, 1, Vent::None, bit(KeyId::LH1)},
    {67, 1, Vent::None, 0ULL},
    {68, 1, Vent::None, bit(KeyId::GSharp)},
    {69, 1, Vent::None, bit(KeyId::AKey)},
    {70, 1, Vent::None, bit(KeyId::Register) | bit(KeyId::AKey)},
    {71, 2, Vent::Register, bit(KeyId::Register) | bit(KeyId::Thumb) | bit(KeyId::LH1) | bit(KeyId::LH2) | bit(KeyId::LH3) | bit(KeyId::LHE) | bit(KeyId::RH1) | bit(KeyId::RH2) | bit(KeyId::RH3)},
    {72, 2, Vent::Register, bit(KeyId::Register) | bit(KeyId::Thumb) | bit(KeyId::LH1) | bit(KeyId::LH2) | bit(KeyId::LH3) | bit(KeyId::RH1) | bit(KeyId::RH2) | bit(KeyId::RH3) | bit(KeyId::RHF)},
    {73, 2, Vent::Register, bit(KeyId::Register) | bit(KeyId::Thumb) | bit(KeyId::LH1) | bit(KeyId::LH2) | bit(KeyId::LH3) | bit(KeyId::LHFSharp) | bit(KeyId::RH1) | bit(KeyId::RH2) | bit(KeyId::RH3)},
    {74, 2, Vent::Register, bit(KeyId::Register) | bit(KeyId::Thumb) | bit(KeyId::LH1) | bit(KeyId::LH2) | bit(KeyId::LH3) | bit(KeyId::RH1) | bit(KeyId::RH2) | bit(KeyId::RH3)},
    {75, 2, Vent::Register, bit(KeyId::Register) | bit(KeyId::Thumb) | bit(KeyId::LH1) | bit(KeyId::LH2) | bit(KeyId::LH3) | bit(KeyId::RH1) | bit(KeyId::RH2) | bit(KeyId::RH3) | bit(KeyId::RHGSharp)},
    {76, 2, Vent::Register, bit(KeyId::Register) | bit(KeyId::Thumb) | bit(KeyId::LH1) | bit(KeyId::LH2) | bit(KeyId::LH3) | bit(KeyId::RH1) | bit(KeyId::RH2)},
    {77, 2, Vent::Register, bit(KeyId::Register) | bit(KeyId::Thumb) | bit(KeyId::LH1) | bit(KeyId::LH2) | bit(KeyId::LH3) | bit(KeyId::RH1)},
    {78, 2, Vent::Register, bit(KeyId::Register) | bit(KeyId::Thumb) | bit(KeyId::LH1) | bit(KeyId::LH2) | bit(KeyId::LH3) | bit(KeyId::RH2)},
    {79, 2, Vent::Register, bit(KeyId::Register) | bit(KeyId::Thumb) | bit(KeyId::LH1) | bit(KeyId::LH2) | bit(KeyId::LH3)},
    {80, 2, Vent::Register, bit(KeyId::Register) | bit(KeyId::Thumb) | bit(KeyId::LH1) | bit(KeyId::LH2) | bit(KeyId::LH3) | bit(KeyId::LHCSharp)},
    {81, 2, Vent::Register, bit(KeyId::Register) | bit(KeyId::Thumb) | bit(KeyId::LH1) | bit(KeyId::LH2)},
    {82, 2, Vent::Register, bit(KeyId::Register) | bit(KeyId::Thumb) | bit(KeyId::LH1) | bit(KeyId::LH2) | bit(KeyId::Side4)},
    {83, 2, Vent::Register, bit(KeyId::Register) | bit(KeyId::Thumb) | bit(KeyId::LH1)},
    {84, 2, Vent::Register, bit(KeyId::Register) | bit(KeyId::Thumb)},
    {85, 3, Vent::Altissimo, bit(KeyId::Register) | bit(KeyId::Thumb) | bit(KeyId::LH2) | bit(KeyId::LH3) | bit(KeyId::RH1) | bit(KeyId::RH2)},
    {86, 3, Vent::Altissimo, bit(KeyId::Register) | bit(KeyId::Thumb) | bit(KeyId::LH2) | bit(KeyId::LH3) | bit(KeyId::RH1) | bit(KeyId::RHGSharp)},
    {87, 3, Vent::Altissimo, bit(KeyId::Register) | bit(KeyId::Thumb) | bit(KeyId::LH2) | bit(KeyId::LH3) | bit(KeyId::RH1) | bit(KeyId::RHBSliver) | bit(KeyId::RHGSharp)},
    {88, 3, Vent::Altissimo, bit(KeyId::Register) | bit(KeyId::Thumb) | bit(KeyId::LH2) | bit(KeyId::LH3) | bit(KeyId::RHGSharp)},
    {89, 3, Vent::Altissimo, bit(KeyId::Register) | bit(KeyId::Thumb) | bit(KeyId::LH2) | bit(KeyId::LH3) | bit(KeyId::LHCSharp) | bit(KeyId::RHGSharp)},
    {90, 3, Vent::Altissimo, bit(KeyId::Register) | bit(KeyId::Thumb) | bit(KeyId::LH2) | bit(KeyId::RHGSharp)},
    {91, 3, Vent::Altissimo, bit(KeyId::Register) | bit(KeyId::Thumb) | bit(KeyId::LH2) | bit(KeyId::RH1) | bit(KeyId::RH2) | bit(KeyId::RHGSharp)},
    {92, 3, Vent::Altissimo, bit(KeyId::Register) | bit(KeyId::Thumb) | bit(KeyId::LH2) | bit(KeyId::LH3) | bit(KeyId::LHFSharp) | bit(KeyId::RH1) | bit(KeyId::RH3)},
    {93, 3, Vent::Altissimo, bit(KeyId::Register) | bit(KeyId::Thumb) | bit(KeyId::LH2) | bit(KeyId::LH3) | bit(KeyId::RHF)},
    {94, 3, Vent::Altissimo, bit(KeyId::Register) | bit(KeyId::Thumb) | bit(KeyId::GSharp) | bit(KeyId::LH2) | bit(KeyId::LH3) | bit(KeyId::LHCSharp) | bit(KeyId::RHGSharp)},
    {95, 3, Vent::Altissimo, bit(KeyId::Register) | bit(KeyId::Thumb) | bit(KeyId::GSharp) | bit(KeyId::LH1) | bit(KeyId::LH2) | bit(KeyId::RH1) | bit(KeyId::RH2)},
    {96, 3, Vent::Altissimo, bit(KeyId::Register) | bit(KeyId::Thumb) | bit(KeyId::GSharp) | bit(KeyId::LH1) | bit(KeyId::RH1) | bit(KeyId::RHFSharp)},
}};

const std::array<Fingering, 45>& table() noexcept {
    static const std::array<Fingering, 45> t = [] {
        std::array<Fingering, 45> a{};
        for (std::size_t i = 0; i < a.size(); ++i) {
            a[i].writtenMidi = kRows[i].written;
            a[i].keys = KeySet(kRows[i].mask);
            a[i].targetRegister = kRows[i].reg;
            a[i].vent = kRows[i].vent;
        }
        return a;
    }();
    return t;
}

/// Non-throwing lookup; nullptr outside [kLowestWritten, kHighestWritten].
const Fingering* find(int written) noexcept {
    if (written < kLowestWritten || written > kHighestWritten) return nullptr;
    return &table()[static_cast<std::size_t>(written - kLowestWritten)];
}

} // namespace

const Fingering& standardFingering(int writtenMidi) {
    const Fingering* f = find(writtenMidi);
    if (f == nullptr) throw std::out_of_range("standardFingering: written note outside E3..C7");
    return *f;
}

std::optional<NoteResolution> resolveNote(int concertMidi, bool overblownMode) noexcept {
    if (!isInRange(concertMidi)) return std::nullopt;
    const int written = concertMidi + kTransposition;
    const Fingering* f = find(written);
    if (f == nullptr) return std::nullopt;
    NoteResolution r;
    r.concertMidi = concertMidi;
    r.fingering = *f;
    r.resonatorWritten = written;
    r.resonatorVent = f->vent;
    if (overblownMode && written >= 71 && written <= 84) {
        const Fingering* low = find(written - 19); // chalumeau fingering a twelfth below, no register key (D-007)
        if (low == nullptr) return std::nullopt;
        r.fingering = *low;
        r.overblownFingering = true;
        r.resonatorWritten = written - 19;
        r.resonatorVent = Vent::None;
    }
    return r;
}

} // namespace clar
