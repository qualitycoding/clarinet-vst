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
inline constexpr double kTiltHz = 600.0;          // tilt shelf corner floor (low enough to move the centroid of low notes)
inline constexpr double kTiltRatio = 1.5;         // corner = max(floor, ratio * note frequency): keeps high notes' centroid movable
inline constexpr double kTiltOverblowDb = 9.0;    // tilt shelf boost at o = 1
inline constexpr double kDynTiltDb = 28.0;        // tilt shelf gain per unit of blowing level (louder = brighter, T-022b)
inline constexpr double kDynTiltRef = 0.6;         // level at which the dynamics tilt is 0 dB (mf)
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

// --- radiation EQ and even-harmonic asymmetry (S-016): fitted to the TinySOL clarinet recordings ---
// Ten peaking bands whose gains are interpolated by blowing level between the pp / mf / ff anchors, and a
// quadratic term y = x + a x^2 whose coefficient a is interpolated the same way (real clarinets radiate weak but
// non-zero even harmonics; the bore model alone produces almost none).
inline constexpr int    kEqBands = 10;
inline constexpr double kEqHz[kEqBands] = {250, 450, 700, 1000, 1400, 2000, 2800, 4000, 5600, 8000};
inline constexpr double kEqQ = 1.2;
inline constexpr double kLevelAnchor[3] = {0.25, 0.6, 0.95};   // blowing level of pp, mf, ff (clar_render velocities)
inline constexpr double kEqDb[3][kEqBands] = {   // fitted by S-016 (logs/S-016-fit.md): residual vs the TinySOL clarinet spectra
    {0, 3.2, -7.5, -3.4, -10.8, -11.6, -4.3, -10.9, -20.8, -25},   // pp
    {0, -0, 2.3, 2.2, 4.5, -1.8, 1.6, -7.3, -6, -18.8},   // mf
    {0, 2.6, -0.6, -0.2, 6.3, 2.8, 2.9, -3.1, -5.2, -12.4}};  // ff
inline constexpr double kEvenAsym[3] = {0.3, 0.3, 0.3};        // quadratic coefficient at pp / mf / ff

// --- decimation ---
inline constexpr double kPassbandFraction = 0.42;  // of the host rate
inline constexpr double kStopbandDb       = 90.0;

} // namespace clar::tuning
