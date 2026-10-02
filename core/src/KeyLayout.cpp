// SPDX-License-Identifier: Apache-2.0
// Normalised drawing geometry for the clarinet picture (D-016). x grows right, y grows down; the
// instrument is upright (mouthpiece at the top). The thumb hole and register key sit in a "rear view"
// inset left of the upper joint. The body axis is x = 0.45.
#include "clar/KeyLayout.h"

namespace clar {
namespace {

constexpr KeyShape shape(KeyId k, KeyShapeKind kind, float x, float y, float w, float h, float rot = 0.0f) {
    return KeyShape{k, kind, x, y, w, h, rot};
}

constexpr float kHole = 0.045f; // finger-hole diameter
constexpr float kAxis = 0.45f - kHole / 2;
using K = KeyId;
using S = KeyShapeKind;

const std::array<KeyShape, kKeyCount> kLayout{{
    shape(K::Register,   S::Pill,      0.100f, 0.180f, 0.045f, 0.030f),
    shape(K::Thumb,      S::RingHole,  0.100f, 0.280f, 0.050f, 0.050f),
    shape(K::AKey,       S::Pill,      0.350f, 0.150f, 0.060f, 0.028f),
    shape(K::GSharp,     S::Pill,      0.350f, 0.205f, 0.055f, 0.028f),
    shape(K::LH1,        S::RingHole,  kAxis,  0.200f, kHole,  kHole),
    shape(K::LH2,        S::RingHole,  kAxis,  0.270f, kHole,  kHole),
    shape(K::LHEbSliver, S::Pill,      0.495f, 0.268f, 0.055f, 0.024f),
    shape(K::LH3,        S::RingHole,  kAxis,  0.340f, kHole,  kHole),
    shape(K::LHCSharp,   S::Plate,     0.330f, 0.400f, 0.055f, 0.030f),
    shape(K::LHE,        S::Plate,     0.395f, 0.425f, 0.055f, 0.030f),
    shape(K::LHF,        S::Plate,     0.330f, 0.450f, 0.055f, 0.030f),
    shape(K::LHFSharp,   S::Plate,     0.395f, 0.470f, 0.055f, 0.030f),
    shape(K::Side1,      S::Pill,      0.540f, 0.500f, 0.050f, 0.026f),
    shape(K::Side2,      S::Pill,      0.540f, 0.535f, 0.050f, 0.026f),
    shape(K::Side3,      S::Pill,      0.540f, 0.570f, 0.050f, 0.026f),
    shape(K::Side4,      S::Pill,      0.600f, 0.605f, 0.050f, 0.026f),
    shape(K::RH1,        S::RingHole,  kAxis,  0.520f, kHole,  kHole),
    shape(K::RH2,        S::RingHole,  kAxis,  0.590f, kHole,  kHole),
    shape(K::RHBSliver,  S::Pill,      0.490f, 0.640f, 0.045f, 0.024f),
    shape(K::RH3,        S::RingHole,  kAxis,  0.660f, kHole,  kHole),
    shape(K::RHE,        S::Plate,     0.500f, 0.715f, 0.055f, 0.030f),
    shape(K::RHF,        S::Plate,     0.565f, 0.735f, 0.055f, 0.030f),
    shape(K::RHFSharp,   S::Plate,     0.500f, 0.760f, 0.055f, 0.030f),
    shape(K::RHGSharp,   S::Plate,     0.565f, 0.790f, 0.055f, 0.030f),
}};

} // namespace

const std::array<KeyShape, kKeyCount>& clarinetKeyLayout() noexcept { return kLayout; }

} // namespace clar
