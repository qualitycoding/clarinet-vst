// SPDX-License-Identifier: Apache-2.0
// STUB (S-000): replaced during implementation. Non-noexcept functions throw NotImplemented;
// noexcept functions return the sentinel documented in the header (plan/DECISIONS.md D-012).
#include "clar/ClarinetVoice.h"
#include "clar/Errors.h"
#include <algorithm>
namespace clar {
struct ClarinetVoice::Impl {};
VoiceParameters clamped(const VoiceParameters& p) noexcept { return p; }
ClarinetVoice::ClarinetVoice(std::shared_ptr<const ResonatorTable>) { throw NotImplemented("ClarinetVoice::ClarinetVoice"); }
ClarinetVoice::~ClarinetVoice() = default;
void ClarinetVoice::prepare(double, int) { throw NotImplemented("ClarinetVoice::prepare"); }
void ClarinetVoice::reset() noexcept {}
void ClarinetVoice::setParameters(const VoiceParameters&) noexcept {}
void ClarinetVoice::noteOn(int, float) noexcept {}
void ClarinetVoice::noteOff(int) noexcept {}
void ClarinetVoice::allNotesOff() noexcept {}
void ClarinetVoice::setBreath(float) noexcept {}
void ClarinetVoice::setPitchBend(float) noexcept {}
void ClarinetVoice::setAftertouch(float) noexcept {}
void ClarinetVoice::process(float* out, int n) noexcept { if (out && n > 0) std::fill(out, out + n, 0.0f); }
KeySet ClarinetVoice::currentKeys() const noexcept { return {}; }
int ClarinetVoice::currentConcertNote() const noexcept { return -1; }
}
