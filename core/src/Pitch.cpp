// SPDX-License-Identifier: Apache-2.0
#include "clar/Pitch.h"
#include <cmath>
#include <stdexcept>

namespace clar {

bool isInRange(int concertMidi) noexcept { return concertMidi >= kLowestConcert && concertMidi <= kHighestConcert; }

int writtenFromConcert(int concertMidi) {
    if (!isInRange(concertMidi)) throw std::out_of_range("writtenFromConcert: concert note outside D3..Bb6");
    return concertMidi + kTransposition;
}

int concertFromWritten(int writtenMidi) {
    if (writtenMidi < kLowestWritten || writtenMidi > kHighestWritten)
        throw std::out_of_range("concertFromWritten: written note outside E3..C7");
    return writtenMidi - kTransposition;
}

double equalTemperedHz(double midi, double a4Hz) noexcept { return a4Hz * std::exp2((midi - 69.0) / 12.0); }

} // namespace clar
