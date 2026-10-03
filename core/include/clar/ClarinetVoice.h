// SPDX-License-Identifier: Apache-2.0
// The real-time monophonic clarinet voice. All methods except the constructor and prepare()
// are real-time safe: no allocation, no locks, no exceptions (D-010).
#pragma once
#include "clar/Fingering.h"
#include "clar/ResonatorTable.h"
#include <atomic>
#include <memory>

namespace clar {

struct VoiceParameters {
    float overblow          = 0.0f;  ///< 0..1  (D-006)
    bool  overblownFingering = false;///< D-007
    float reedHardness      = 0.5f;  ///< 0..1
    float brightness        = 0.5f;  ///< 0..1 (dark .. bright)
    float breathNoise       = 0.3f;  ///< 0..1
    float vibratoRateHz     = 5.5f;  ///< 3..8
    float vibratoDepth      = 0.0f;  ///< 0..1 (scaled by aftertouch, D-011)
    float portamentoMs      = 30.0f; ///< 0..500
    float tuningA4Hz        = 440.0f;///< 415..466
    float outputGainDb      = 0.0f;  ///< -24..+12
    friend bool operator==(const VoiceParameters&, const VoiceParameters&) = default;
};

/// Clamp every field into its documented range; NaN -> default. Stub sentinel: returns input unchanged.
VoiceParameters clamped(const VoiceParameters& p) noexcept;

class ClarinetVoice {
public:
    explicit ClarinetVoice(std::shared_ptr<const ResonatorTable> table);
    ~ClarinetVoice();

    /// Allocates. Must be called before process(). Throws std::invalid_argument for
    /// sampleRate outside [22050, 192000] or maxBlockSize < 1.
    void prepare(double sampleRate, int maxBlockSize);
    void reset() noexcept;

    void setParameters(const VoiceParameters& p) noexcept; ///< values are clamped()
    void noteOn(int concertMidi, float velocity) noexcept;  ///< out-of-range notes ignored
    void noteOff(int concertMidi) noexcept;
    void allNotesOff() noexcept;
    void setBreath(float amount01) noexcept;       ///< CC2/CC11; <0 means "no breath controller"
    void setPitchBend(float minus1to1) noexcept;    ///< +-2 semitones
    void setAftertouch(float amount01) noexcept;

    /// Renders numSamples (<= maxBlockSize) mono samples into out, |out| <= 1.
    void process(float* out, int numSamples) noexcept;

    /// Latency in host samples introduced by the oversampling decimator (valid after prepare()).
    int latencySamples() const noexcept;

    /// For the UI thread (lock-free). Empty set and -1 when silent / released.
    KeySet currentKeys() const noexcept;
    int currentConcertNote() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace clar
