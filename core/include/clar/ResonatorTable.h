// SPDX-License-Identifier: Apache-2.0
// Modal description of the bore input impedance per (written note, vent) (Colinot et al. 2021, eq. 11).
#pragma once
#include "clar/Fingering.h"
#include <complex>
#include <cstddef>
#include <string_view>
#include <vector>

namespace clar {

struct Mode {
    std::complex<double> pole;    ///< s_n in rad/s, Re < 0
    std::complex<double> residue; ///< C_n, dimensionless impedance Z/Zc
};

struct ResonatorParams {
    std::vector<Mode> modes;      ///< 2 <= size <= 24, sorted by Im(pole) ascending
    double tuningScale = 1.0;     ///< D-008 per-entry calibration factor in [0.8, 1.25]
};

/// Immutable table loaded from data/clarinet_resonators.json (schema "clarinet-vst/resonators@1",
/// documented in plan/DECISIONS.md D-009).
class ResonatorTable {
public:
    /// Throws ParseError on malformed JSON, wrong schema, unknown vent, unstable pole (Re >= 0),
    /// unsorted modes, mode count outside [2,24], tuning_scale outside [0.8,1.25], non-finite numbers,
    /// duplicate (written, vent) entries, or input larger than 4 MiB.
    static ResonatorTable fromJson(std::string_view json);

    /// Throws std::out_of_range if the (writtenMidi, vent) pair is absent.
    const ResonatorParams& lookup(int writtenMidi, Vent vent) const;

    std::size_t size() const noexcept;

private:
    struct Entry { int written; Vent vent; ResonatorParams params; };
    std::vector<Entry> entries_;
};

/// Embedded copy of data/clarinet_resonators.json compiled into the binary (generated at build time).
/// Throws NotImplemented until S-007.
std::string_view embeddedResonatorJson();

} // namespace clar
