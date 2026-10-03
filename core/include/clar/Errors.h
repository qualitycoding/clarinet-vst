// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <stdexcept>
#include <string>

namespace clar {

/// Thrown by every not-yet-implemented, non-noexcept stub (see plan/DECISIONS.md D-012).
struct NotImplemented : std::logic_error {
    explicit NotImplemented(const std::string& what) : std::logic_error("not implemented: " + what) {}
};

/// Thrown when a data file (resonator table, fingering table) is malformed.
struct ParseError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

} // namespace clar
