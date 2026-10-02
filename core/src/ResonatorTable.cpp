// SPDX-License-Identifier: Apache-2.0
// STUB (S-000): replaced during implementation. Non-noexcept functions throw NotImplemented;
// noexcept functions return the sentinel documented in the header (plan/DECISIONS.md D-012).
#include "clar/ResonatorTable.h"
#include "clar/Errors.h"
namespace clar {
ResonatorTable ResonatorTable::fromJson(std::string_view) { throw NotImplemented("ResonatorTable::fromJson"); }
const ResonatorParams& ResonatorTable::lookup(int, Vent) const { throw NotImplemented("ResonatorTable::lookup"); }
std::size_t ResonatorTable::size() const noexcept { return 0; }
std::string_view embeddedResonatorJson() { throw NotImplemented("embeddedResonatorJson"); }
}
