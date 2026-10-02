// SPDX-License-Identifier: Apache-2.0
// Concert/written pitch handling for the B-flat soprano clarinet. MIDI note numbers, C4 = 60.
#pragma once

namespace clar {

inline constexpr int kLowestConcert  = 50; ///< D3, written E3 (C-024)
inline constexpr int kHighestConcert = 94; ///< Bb6, written C7 (A-003, C-024)
inline constexpr int kTransposition  = 2;  ///< written = concert + 2 (major second, C-024)
inline constexpr int kLowestWritten  = kLowestConcert  + kTransposition; // 52
inline constexpr int kHighestWritten = kHighestConcert + kTransposition; // 96

/// true iff concertMidi is playable with a standard fingering. Stub sentinel: false.
bool isInRange(int concertMidi) noexcept;

/// Written (transposed) MIDI note. Throws std::out_of_range if !isInRange(concertMidi).
int writtenFromConcert(int concertMidi);

/// Concert MIDI note for a written note. Throws std::out_of_range outside [52, 96].
int concertFromWritten(int writtenMidi);

/// 12-TET frequency in Hz: a4Hz * 2^((midi-69)/12). Stub sentinel: 0.0.
double equalTemperedHz(double midi, double a4Hz = 440.0) noexcept;

} // namespace clar
