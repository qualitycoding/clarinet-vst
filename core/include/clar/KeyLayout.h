// SPDX-License-Identifier: Apache-2.0
// Geometry of the drawn clarinet keys, rings and holes, in normalised editor coordinates
// (0..1, origin top-left). The two rear levers (register key, thumb hole) are drawn in a "rear view"
// inset beside the body (D-016).
#pragma once
#include "clar/Keys.h"
#include <array>

namespace clar {

enum class KeyShapeKind : std::uint8_t { RingHole, PlainHole, Pill, Plate };

struct KeyShape {
    KeyId key = KeyId::Count;
    KeyShapeKind kind = KeyShapeKind::PlainHole;
    float x = 0, y = 0, w = 0, h = 0;   ///< bounding box, all within [0,1]
    float rotationDeg = 0;
};

/// One entry per KeyId, in KeyId order. Stub sentinel: all entries default-constructed.
const std::array<KeyShape, kKeyCount>& clarinetKeyLayout() noexcept;

} // namespace clar
