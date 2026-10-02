// SPDX-License-Identifier: Apache-2.0
// Tuning constants of the real-time voice (plan/DECISIONS.md D-006, D-011, D-014). NOT frozen: S-016 may
// retune them within the decision rules. Every value has a one-line meaning.
#pragma once

namespace clar::tuning {

// --- exciter (D-011) ---
inline constexpr double kGammaVelBase    = 0.45;  // blowing pressure at velocity 0 (breath controller absent)
inline constexpr double kGammaVelSlope   = 0.30;  // + slope * velocity (C-007: 0.5 piano .. 0.7 forte)
inline constexpr double kGammaBreathBase = 0.43;  // blowing pressure at breath 0+ (breath controller present)
inline constexpr double kGammaBreathSlope = 0.32; // + slope * breath
inline constexpr double kBreathFloor     = 0.02;  // breath below this = no air
inline constexpr double kGammaMax        = 0.95;  // gamma >= 1 closes the reed permanently (C-009)
inline constexpr double kZetaCentre      = 0.40;  // reed-channel parameter at reedHardness = 0.5 (spike-verified, C-007)
inline constexpr double kZetaSpan        = 0.15;  // + span * (0.5 - hardness): harder reed -> smaller zeta
inline constexpr double kZetaMin         = 0.25;
inline constexpr double kZetaMax         = 0.45;
inline constexpr double kQrBase          = 0.70;  // lip damping: avoids squeaks at normal playing (C-022)
inline constexpr double kQrOverblowDrop  = 0.20;  // tighter embouchure with Overblow (0.40 left 5/14 low notes in R1 at o = 1, S-011)
inline constexpr double kQrMin           = 0.25;
inline constexpr double kAttackTauBase   = 0.008; // s, tanh ramp time constant (spikes C4/C9: 3..30 ms robust)
inline constexpr double kAttackTauShrink = 0.80;  // attack shortened by up to this fraction with Overblow
inline constexpr double kReedFreqMinHz   = 1500.0;// reed resonance floor (C-026)
inline constexpr double kReedFreqRatio   = 2.0;   // f_r = max(floor, ratio * f_note) (spike C7, C-032)
inline constexpr double kSmoothSeconds   = 0.010; // one-pole parameter smoothing
inline constexpr double kReleaseSeconds  = 0.040; // gamma ramp to zero after the last note-off
inline constexpr double kVibratoMaxCents = 15.0;

// --- Overblow (D-006) ---
inline constexpr double kOverblowGammaGain = 0.35; // gamma *= 1 + gain * o
inline constexpr double kKappaStart  = 0.6;        // mode-1 damping starts above this Overblow value
inline constexpr double kKappaMax    = 10.0;       // x10 first-mode bandwidth at o = 1 (C-033)
inline constexpr double kKappaForced = 10.0;       // overblown-fingering switch (D-007)
inline constexpr double kShelfOverblowDb = 3.0;    // brightness shelf boost at o = 1
inline constexpr double kTiltHz = 600.0;          // Overblow tilt shelf corner (low enough to move the centroid of low notes)
inline constexpr double kTiltOverblowDb = 9.0;    // tilt shelf boost at o = 1
inline constexpr double kNoiseOverblowGain = 1.0;  // breath-noise gain *= 1 + gain * o

// --- output stage (D-014) ---
inline constexpr double kHighPassHz   = 100.0;
inline constexpr double kLatticeHz    = 1500.0, kLatticeQ = 0.7, kLatticeDb = 3.0; // reinforced band (C-015)
inline constexpr double kBellHz       = 2100.0, kBellQ = 2.0,    kBellDb = 2.0;    // bell resonance (C-015)
inline constexpr double kShelfHz      = 3000.0;
inline constexpr double kShelfDbMin   = -6.0, kShelfDbSpan = 9.0;                 // -6 dB + 9 dB * brightness
inline constexpr double kNoiseGain    = 0.05;
inline constexpr double kNoiseHighPassHz = 1000.0, kNoiseLowPassHz = 6000.0;
inline constexpr double kDcBlockHz    = 5.0;
inline constexpr double kOutputScale  = 0.9;       // pressure -> pre-tanh level

// --- decimation ---
inline constexpr double kPassbandFraction = 0.42;  // of the host rate
inline constexpr double kStopbandDb       = 90.0;

} // namespace clar::tuning
