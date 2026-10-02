// SPDX-License-Identifier: Apache-2.0
// STUB (S-000): replaced during implementation. Non-noexcept functions throw NotImplemented;
// noexcept functions return the sentinel documented in the header (plan/DECISIONS.md D-012).
#include "clar/KeyLayout.h"
namespace clar {
const std::array<KeyShape, kKeyCount>& clarinetKeyLayout() noexcept {
    static const std::array<KeyShape, kKeyCount> empty{};
    return empty;
}
}
