// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "clar/Keys.h"
#include <optional>

namespace clar {

/// Which virtual resonator family a fingering uses (D-009):
/// None = closed register hole (chalumeau + throat), Register = register hole open (clarion, twelfth),
/// Altissimo = vented short-resonator approximation (D-009 altissimo rule).
enum class Vent : std::uint8_t { None, Register, Altissimo };

struct Fingering {
    int writtenMidi = -1;
    KeySet keys;               ///< levers pressed / holes covered (highlighted in the UI)
    int targetRegister = 1;    ///< 1 chalumeau/throat, 2 clarion (twelfth), 3 altissimo
    Vent vent = Vent::None;
    friend bool operator==(const Fingering&, const Fingering&) = default;
};

/// Standard fingering for written notes 52..96 per D-004 / tests/fixtures/clarinet_standard_fingerings.txt.
/// Throws std::out_of_range outside [52, 96].
const Fingering& standardFingering(int writtenMidi);

/// Result of mapping an incoming concert MIDI note to what the virtual player does.
struct NoteResolution {
    int concertMidi = -1;            ///< the pitch that should sound
    Fingering fingering;             ///< what the UI shows
    bool overblownFingering = false; ///< true: fingering is a twelfth below, no register key; model driven to its 2nd register
    int resonatorWritten = -1;       ///< resonator table key (written note) used by the voice
    Vent resonatorVent = Vent::None; ///< resonator table key (vent) used by the voice
    friend bool operator==(const NoteResolution&, const NoteResolution&) = default;
};

/// D-007. overblownMode=false: standard fingering of the note; resonator = (written, fingering.vent).
/// overblownMode=true and written in [71, 84] (concert 69..82): fingering = standardFingering(written-19)
/// (no register key), overblownFingering=true, resonator = (written-19, Vent::None).
/// Otherwise identical to overblownMode=false. Out of range: std::nullopt (also the stub sentinel).
std::optional<NoteResolution> resolveNote(int concertMidi, bool overblownMode) noexcept;

} // namespace clar
