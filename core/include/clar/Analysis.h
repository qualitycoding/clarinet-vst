// SPDX-License-Identifier: Apache-2.0
// Offline signal analysis used by tests and tools (never on the audio thread).
#pragma once
#include <span>
#include <vector>

namespace clar {

enum class Regime { Silent, FirstRegister, Twelfth, Other };

/// YIN fundamental estimate over the whole span (threshold 0.1, search 50-2500 Hz). Returns 0 for
/// silence (RMS < 1e-4) or no periodicity. Stub sentinel: -1.
double estimateF0(std::span<const float> x, double sampleRate) noexcept;

/// Uses the last half of x. Silent if RMS < 1e-4; FirstRegister if f0/expectedR1Hz in [0.94, 1.06];
/// Twelfth (second register of a cylindrical bore, ~3x) if in [2.82, 3.18]; else Other.
/// Stub sentinel: Other.
Regime classifyRegime(std::span<const float> x, double sampleRate, double expectedR1Hz) noexcept;

/// Levels (dB re. the strongest) of harmonics 1..n measured with a Hann-windowed FFT over x.
/// Throws std::invalid_argument for f0 <= 0 or n < 1.
std::vector<double> harmonicLevelsDb(std::span<const float> x, double sampleRate, double f0, int n);

/// Power-spectrum centroid in Hz. Stub sentinel: -1.
double spectralCentroidHz(std::span<const float> x, double sampleRate) noexcept;

/// Cents between two frequencies, 1200*log2(a/b). Stub sentinel: NaN.
double cents(double a, double b) noexcept;

} // namespace clar
