// SPDX-License-Identifier: Apache-2.0
// Real-time monophonic clarinet voice (D-005, D-006, D-007, D-009, D-010, D-011, D-014).
// Single-reed exciter (C-006) coupled to the modal resonator of the sounding fingering, integrated at an
// oversampled internal rate (>= 176.4 kHz, C-008), decimated by cascaded half-band FIRs, then the output
// stage. After prepare() nothing in process()/note/parameter calls allocates, locks or throws.
#include "clar/ClarinetVoice.h"
#include "VoiceTuning.h"
#include "clar/Pitch.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <complex>
#include <cstdint>
#include <numbers>
#include <stdexcept>
#include <vector>

namespace clar {
namespace {

constexpr int kMaxModes = 24;
constexpr int kMaxHeld = 16;
constexpr int kCtrl = 16;                 // host samples per control step
constexpr double kPi = std::numbers::pi;

float sanitize(float v, float lo, float hi, float def) noexcept {
    return std::isfinite(v) ? std::clamp(v, lo, hi) : def;
}
double sgn(double v) noexcept { return v > 0.0 ? 1.0 : (v < 0.0 ? -1.0 : 0.0); }
double smoothstep(double a, double b, double x) noexcept {
    const double t = std::clamp((x - a) / (b - a), 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

double besselI0(double x) noexcept {
    double sum = 1.0, term = 1.0;
    for (int k = 1; k < 60; ++k) {
        const double h = x / (2.0 * k);
        term *= h * h;
        sum += term;
        if (term < 1e-18 * sum) break;
    }
    return sum;
}

/// Linear-phase half-band FIR decimator by 2 (Kaiser window); the stored taps are the odd offsets.
class HalfBand {
public:
    void design(int taps, double beta) {
        n_ = taps;
        c_ = (taps - 1) / 2;
        h_.assign(static_cast<std::size_t>(c_ + 1), 0.0);
        const double i0b = besselI0(beta);
        double sum = 0.5;
        for (int j = 1; j <= c_; j += 2) {
            const double r = static_cast<double>(j) / static_cast<double>(c_);
            const double w = besselI0(beta * std::sqrt(std::max(0.0, 1.0 - r * r))) / i0b;
            const double v = ((j % 4 == 1) ? 1.0 : -1.0) / (kPi * j) * w;
            h_[static_cast<std::size_t>(j)] = v;
            sum += 2.0 * v;
        }
        for (auto& v : h_) v /= sum;
        center_ = 0.5 / sum;
        hist_.assign(static_cast<std::size_t>(2 * n_), 0.0);
        pos_ = 0;
    }
    void reset() noexcept { std::fill(hist_.begin(), hist_.end(), 0.0); pos_ = 0; }
    int centre() const noexcept { return c_; }
    double push2(double a, double b) noexcept {
        push(a);
        push(b);
        const double* w = hist_.data() + pos_;
        double y = center_ * w[c_];
        for (int j = 1; j <= c_; j += 2) y += h_[static_cast<std::size_t>(j)] * (w[c_ - j] + w[c_ + j]);
        return y;
    }
private:
    void push(double v) noexcept {
        if (--pos_ < 0) pos_ = n_ - 1;
        hist_[static_cast<std::size_t>(pos_)] = v;
        hist_[static_cast<std::size_t>(pos_ + n_)] = v;
    }
    int n_ = 0, c_ = 0, pos_ = 0;
    double center_ = 0.5;
    std::vector<double> h_, hist_;
};

struct Biquad {
    double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;
    void reset() noexcept { z1 = z2 = 0.0; }
    double process(double x) noexcept {
        const double y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return y;
    }
    void set(double B0, double B1, double B2, double A0, double A1, double A2) noexcept {
        b0 = B0 / A0; b1 = B1 / A0; b2 = B2 / A0; a1 = A1 / A0; a2 = A2 / A0;
    }
    void peaking(double fs, double f0, double q, double dB) noexcept {
        const double A = std::pow(10.0, dB / 40.0), w0 = 2.0 * kPi * f0 / fs, al = std::sin(w0) / (2.0 * q), c = std::cos(w0);
        set(1.0 + al * A, -2.0 * c, 1.0 - al * A, 1.0 + al / A, -2.0 * c, 1.0 - al / A);
    }
    void highShelf(double fs, double f0, double dB) noexcept {
        const double A = std::pow(10.0, dB / 40.0), w0 = 2.0 * kPi * f0 / fs, c = std::cos(w0), s = std::sin(w0);
        const double al = s / std::sqrt(2.0), t = 2.0 * std::sqrt(A) * al;
        set(A * ((A + 1) + (A - 1) * c + t), -2.0 * A * ((A - 1) + (A + 1) * c), A * ((A + 1) + (A - 1) * c - t),
            (A + 1) - (A - 1) * c + t, 2.0 * ((A - 1) - (A + 1) * c), (A + 1) - (A - 1) * c - t);
    }
};

struct OnePoleHP {   // y = a (y1 + x - x1)
    double a = 0.99, x1 = 0, y1 = 0;
    void setCutoff(double fs, double fc) noexcept { a = 1.0 / (1.0 + 2.0 * kPi * fc / fs); }
    void reset() noexcept { x1 = y1 = 0.0; }
    double process(double x) noexcept { y1 = a * (y1 + x - x1); x1 = x; return y1; }
};
/// Piecewise-linear interpolation of v[0..2] (pp, mf, ff anchors) at blowing level `level`, clamped at the ends.
inline double levelInterp(double level, const double v[3]) noexcept {
    const double* a = tuning::kLevelAnchor;
    if (level <= a[0]) return v[0];
    if (level >= a[2]) return v[2];
    if (level <= a[1]) return v[0] + (v[1] - v[0]) * (level - a[0]) / (a[1] - a[0]);
    return v[1] + (v[2] - v[1]) * (level - a[1]) / (a[2] - a[1]);
}

struct OnePoleLP {
    double a = 0.5, y1 = 0;
    void setCutoff(double fs, double fc) noexcept { a = 1.0 - std::exp(-2.0 * kPi * fc / fs); }
    void reset() noexcept { y1 = 0.0; }
    double process(double x) noexcept { y1 += a * (x - y1); return y1; }
};

struct ModeSet {
    int n = 0;
    double re[kMaxModes] = {}, im[kMaxModes] = {}, cr[kMaxModes] = {}, ci[kMaxModes] = {};
};

} // namespace

VoiceParameters clamped(const VoiceParameters& p) noexcept {
    const VoiceParameters d;
    VoiceParameters r;
    r.overblow           = sanitize(p.overblow, 0.0f, 1.0f, d.overblow);
    r.overblownFingering = p.overblownFingering;
    r.reedHardness       = sanitize(p.reedHardness, 0.0f, 1.0f, d.reedHardness);
    r.brightness         = sanitize(p.brightness, 0.0f, 1.0f, d.brightness);
    r.breathNoise        = sanitize(p.breathNoise, 0.0f, 1.0f, d.breathNoise);
    r.vibratoRateHz      = sanitize(p.vibratoRateHz, 3.0f, 8.0f, d.vibratoRateHz);
    r.vibratoDepth       = sanitize(p.vibratoDepth, 0.0f, 1.0f, d.vibratoDepth);
    r.portamentoMs       = sanitize(p.portamentoMs, 0.0f, 500.0f, d.portamentoMs);
    r.tuningA4Hz         = sanitize(p.tuningA4Hz, 415.0f, 466.0f, d.tuningA4Hz);
    r.outputGainDb       = sanitize(p.outputGainDb, -24.0f, 12.0f, d.outputGainDb);
    return r;
}

struct ClarinetVoice::Impl {
    explicit Impl(std::shared_ptr<const ResonatorTable> t) : table(std::move(t)) {}

    // ---- configuration ----
    std::shared_ptr<const ResonatorTable> table;
    const ResonatorParams* lookup[kHighestWritten - kLowestWritten + 1][3] = {};
    bool prepared = false;
    double fs = 48000.0, dtInt = 1.0 / 192000.0;
    int K = 4, nStages = 0, latency = 0;
    std::array<HalfBand, 3> stages;

    // ---- parameters and performance state ----
    VoiceParameters target;
    double smLevel = 0.6, smOverblow = 0, smBright = 0.5, smNoise = 0.3, smHardness = 0.5, smVibDepth = 0, smGainDb = 0;
    float velocity = 0.7f, breath = -1.0f, bend = 0.0f, aftertouch = 0.0f;
    int held[kMaxHeld] = {};
    int nHeld = 0;
    int sounding = -1;
    bool gate = false, active = false;
    NoteResolution res;

    // ---- modal sets and blending ----
    ModeSet oldSet, newSet, eff;
    double blend = 1.0, blendDur = 0.003;
    double ftOld = 220.0, ftNew = 220.0;     // concert note frequency at A4 = 440 (reed resonance rule)
    bool forcedKappa = false, registerEntry = false;
    int M = 0;

    // ---- DSP state ----
    alignas(32) double pnr[kMaxModes] = {}, pni[kMaxModes] = {}, Er[kMaxModes] = {}, Ei[kMaxModes] = {};
    alignas(32) double Gr[kMaxModes] = {}, Gi[kMaxModes] = {};
    double x = 0.0, xp = 0.0;
    double c1 = 0.0, c2 = 0.0, invDen = 1.0, reedEta = 0.01, reedKc = 100.0, zeta = 0.4;
    double lastPitch = -1.0, lastKappa = -1.0;
    bool setsDirty = true;

    // ---- envelopes ----
    double attackT = 0.0, attackTau = 0.008, gSm = 0.0, relGain = 1.0, velGain = 1.0, quietT = 0.0;
    bool inAttack = false;
    double gammaStart = 0.0, gammaEnd = 0.0;
    double vibPhase = 0.0;

    // ---- output stage ----
    OnePoleHP hp, noiseHp, dcBlock;
    OnePoleLP noiseLp;
    Biquad lattice, bell, shelf, tilt;
    Biquad eq[tuning::kEqBands];
    double lastEqLevel = -1.0, evenCoef = 0.0;
    double lastShelfDb = 1e9, lastTiltDb = 1e9, lastTiltHz = 0.0;
    std::uint32_t noiseState = 0x5A5A5A5Au;
    double flowAvg = 0.0;

    // ---- UI (lock-free) ----
    std::atomic<std::uint32_t> uiKeys{0};
    std::atomic<int> uiNote{-1};

    // ------------------------------------------------------------------------------------------
    void prepare(double sampleRate, int maxBlock) {
        if (!(sampleRate >= 22050.0 && sampleRate <= 192000.0) || maxBlock < 1)
            throw std::invalid_argument("ClarinetVoice::prepare: sampleRate must be in [22050, 192000], maxBlockSize >= 1");
        fs = sampleRate;
        int k = 1;
        while (static_cast<double>(k) * fs < 176400.0 - 1e-9 && k < 8) k *= 2;
        K = k;
        dtInt = 1.0 / (fs * K);
        for (int w = kLowestWritten; w <= kHighestWritten; ++w)
            for (int v = 0; v < 3; ++v) {
                const ResonatorParams* p = nullptr;
                try { p = &table->lookup(w, static_cast<Vent>(v)); } catch (...) { p = nullptr; }
                lookup[w - kLowestWritten][v] = p;
            }
        nStages = 0;
        for (int m = 1; m < K; m *= 2) ++nStages;
        double latSamples = 0.0;
        const double fpass = tuning::kPassbandFraction * fs;
        const double beta = 0.1102 * (tuning::kStopbandDb - 8.7);
        for (int s = 0; s < nStages; ++s) {
            const double fin = fs * K / static_cast<double>(1 << s), fout = fin / 2.0;
            const double delta = std::max((fout - 2.0 * fpass) / fin, 0.05);
            int taps = static_cast<int>(std::ceil((tuning::kStopbandDb - 8.0) / (2.285 * 2.0 * kPi * delta))) + 1;
            taps = std::max(11, taps);
            while (taps % 4 != 3) ++taps;
            stages[static_cast<std::size_t>(s)].design(taps, beta);
            latSamples += static_cast<double>(stages[static_cast<std::size_t>(s)].centre()) / (static_cast<double>(K) / static_cast<double>(1 << s));
        }
        latency = static_cast<int>(std::ceil(latSamples));
        hp.setCutoff(fs, tuning::kHighPassHz);
        noiseHp.setCutoff(fs, tuning::kNoiseHighPassHz);
        noiseLp.setCutoff(fs, std::min(tuning::kNoiseLowPassHz, 0.45 * fs));
        dcBlock.setCutoff(fs, tuning::kDcBlockHz);
        lattice.peaking(fs, tuning::kLatticeHz, tuning::kLatticeQ, tuning::kLatticeDb);
        bell.peaking(fs, tuning::kBellHz, tuning::kBellQ, tuning::kBellDb);
        prepared = true;
        resetAll();
    }

    void resetAll() noexcept {
        nHeld = 0; sounding = -1; gate = false; active = false;
        velocity = 0.7f; breath = -1.0f; bend = 0.0f; aftertouch = 0.0f;
        smOverblow = target.overblow; smBright = target.brightness; smNoise = target.breathNoise;
        smHardness = target.reedHardness; smVibDepth = target.vibratoDepth; smGainDb = target.outputGainDb;
        clearDsp();
        publish();
    }

    void clearDsp() noexcept {
        for (int i = 0; i < kMaxModes; ++i) pnr[i] = pni[i] = 0.0;
        x = xp = 0.0;
        M = 0; blend = 1.0; setsDirty = true; lastPitch = -1.0; lastKappa = -1.0;
        attackT = 0.0; inAttack = false; gSm = 0.0; relGain = 1.0; velGain = 1.0; quietT = 0.0;
        gammaStart = gammaEnd = 0.0; vibPhase = 0.0; flowAvg = 0.0;
        hp.reset(); noiseHp.reset(); noiseLp.reset(); dcBlock.reset();
        lattice.reset(); bell.reset(); shelf.reset(); tilt.reset(); for (auto& e : eq) e.reset(); lastShelfDb = 1e9; lastTiltDb = 1e9; lastTiltHz = 0.0;
        for (auto& s : stages) s.reset();
        noiseState = 0x5A5A5A5Au;
    }

    void publish() noexcept {
        if (sounding >= 0) {
            uiKeys.store(static_cast<std::uint32_t>(res.fingering.keys.to_ulong()), std::memory_order_relaxed);
            uiNote.store(sounding, std::memory_order_relaxed);
        } else {
            uiKeys.store(0, std::memory_order_relaxed);
            uiNote.store(-1, std::memory_order_relaxed);
        }
    }

    // ---- note handling ------------------------------------------------------------------------
    bool buildSet(int concert, ModeSet& out, NoteResolution& r) noexcept {
        const auto rr = resolveNote(concert, target.overblownFingering);
        if (!rr) return false;
        const int wi = rr->resonatorWritten - kLowestWritten;
        if (wi < 0 || wi > kHighestWritten - kLowestWritten) return false;
        const ResonatorParams* p = lookup[wi][static_cast<int>(rr->resonatorVent)];
        if (p == nullptr) return false;
        out.n = static_cast<int>(std::min<std::size_t>(p->modes.size(), kMaxModes));
        for (int i = 0; i < out.n; ++i) {
            const Mode& m = p->modes[static_cast<std::size_t>(i)];
            out.re[i] = m.pole.real();
            out.im[i] = m.pole.imag() * p->tuningScale;
            out.cr[i] = m.residue.real();
            out.ci[i] = m.residue.imag();
        }
        r = *rr;
        return true;
    }

    void startNote(int concert) noexcept {
        ModeSet fresh;
        NoteResolution r;
        if (!buildSet(concert, fresh, r)) return;
        const bool legato = gate && active && sounding >= 0;
        const double ft = equalTemperedHz(concert, 440.0);
        if (legato) {
            oldSet = eff;
            ftOld = currentBaseFreq();
            newSet = fresh;
            ftNew = ft;
            blend = 0.0;
            blendDur = std::max(0.003, static_cast<double>(target.portamentoMs) * 1e-3);
        } else {
            newSet = fresh; oldSet = fresh; ftOld = ftNew = ft;
            blend = 1.0;
            if (!active) {                       // from silence: clean start
                const double keepSmooth = gSm; (void)keepSmooth;
                for (int i = 0; i < kMaxModes; ++i) pnr[i] = pni[i] = 0.0;
                x = xp = 0.0; M = 0;
                for (auto& s : stages) s.reset();
                hp.reset(); dcBlock.reset(); lattice.reset(); bell.reset(); shelf.reset(); tilt.reset(); for (auto& e : eq) e.reset();
                noiseHp.reset(); noiseLp.reset();
            }
            attackT = 0.0; inAttack = true; relGain = 1.0;
            attackTau = tuning::kAttackTauBase * (1.0 - tuning::kAttackTauShrink * smOverblow);
            gSm = gammaTarget();
            gammaEnd = 0.0; gammaStart = 0.0;
        }
        res = r;
        forcedKappa = r.overblownFingering;
        registerEntry = (r.resonatorVent == Vent::Register);
        sounding = concert;
        gate = true; active = true; quietT = 0.0;
        setsDirty = true;
        publish();
    }

    double currentBaseFreq() const noexcept {
        const double b = std::clamp(blend, 0.0, 1.0);
        return ftOld * std::pow(ftNew / ftOld, b);
    }

    double gammaTarget() const noexcept {
        double g;
        if (breath >= 0.0f) g = breath < static_cast<float>(tuning::kBreathFloor) ? 0.0 : tuning::kGammaBreathBase + tuning::kGammaBreathSlope * static_cast<double>(breath);
        else g = tuning::kGammaVelBase + tuning::kGammaVelSlope * static_cast<double>(velocity);
        if (g <= 0.0) return 0.0;
        return std::min(tuning::kGammaMax, g * (1.0 + tuning::kOverblowGammaGain * smOverblow));
    }

    void noteOn(int concert, float vel) noexcept {
        if (!prepared || !isInRange(concert)) return;
        velocity = std::clamp(std::isfinite(vel) ? vel : 0.7f, 0.0f, 1.0f);
        for (int i = 0; i < nHeld; ++i)
            if (held[i] == concert) { for (int j = i; j + 1 < nHeld; ++j) held[j] = held[j + 1]; --nHeld; break; }
        if (nHeld == kMaxHeld) { for (int j = 0; j + 1 < nHeld; ++j) held[j] = held[j + 1]; --nHeld; }
        held[nHeld++] = concert;
        startNote(concert);
    }

    void noteOff(int concert) noexcept {
        if (!prepared) return;
        bool found = false;
        for (int i = 0; i < nHeld; ++i)
            if (held[i] == concert) { for (int j = i; j + 1 < nHeld; ++j) held[j] = held[j + 1]; --nHeld; found = true; break; }
        if (!found) return;
        if (nHeld > 0) { if (held[nHeld - 1] != sounding) startNote(held[nHeld - 1]); }
        else gate = false;
    }

    void goIdle() noexcept {
        active = false; gate = false; sounding = -1; nHeld = 0;
        for (int i = 0; i < kMaxModes; ++i) pnr[i] = pni[i] = 0.0;
        x = xp = 0.0; M = 0; gSm = 0.0; gammaStart = gammaEnd = 0.0;
        for (auto& s : stages) s.reset();
        hp.reset(); dcBlock.reset(); lattice.reset(); bell.reset(); shelf.reset(); tilt.reset(); for (auto& e : eq) e.reset(); noiseHp.reset(); noiseLp.reset();
        publish();
    }

    void setParams(const VoiceParameters& p) noexcept {
        const bool fingeringChanged = clamped(p).overblownFingering != target.overblownFingering;
        target = clamped(p);
        if (fingeringChanged && active && gate && sounding >= 0) startNote(sounding);
    }

    // ---- control rate -------------------------------------------------------------------------
    void setShelf() noexcept {
        const double dB = tuning::kShelfDbMin + tuning::kShelfDbSpan * smBright + tuning::kShelfOverblowDb * smOverblow;
        if (std::fabs(dB - lastShelfDb) > 0.01) { shelf.highShelf(fs, tuning::kShelfHz, dB); lastShelfDb = dB; }
        if (std::fabs(smLevel - lastEqLevel) > 0.002) {                 // radiation EQ + asymmetry follow the blowing level
            lastEqLevel = smLevel;
            for (int i = 0; i < tuning::kEqBands; ++i) {
                double db[3] = {tuning::kEqDb[0][i], tuning::kEqDb[1][i], tuning::kEqDb[2][i]};
                if (tuning::kEqHz[i] < 0.45 * fs) eq[i].peaking(fs, tuning::kEqHz[i], tuning::kEqQ, levelInterp(smLevel, db));
                else eq[i] = Biquad{};
            }
            const double a[3] = {tuning::kEvenAsym[0], tuning::kEvenAsym[1], tuning::kEvenAsym[2]};
            evenCoef = levelInterp(smLevel, a);
        }
        const double tiltDb = tuning::kTiltOverblowDb * smOverblow + tuning::kDynTiltDb * (smLevel - tuning::kDynTiltRef);
        const double tiltHz = std::min(0.4 * fs, std::max(tuning::kTiltHz, tuning::kTiltRatio * currentBaseFreq()));
        if (std::fabs(tiltDb - lastTiltDb) > 0.01 || std::fabs(tiltHz / std::max(lastTiltHz, 1.0) - 1.0) > 0.01) {
            tilt.highShelf(fs, tiltHz, tiltDb); lastTiltDb = tiltDb; lastTiltHz = tiltHz;
        }
    }

    void rebuildEffective() noexcept {
        if (blend >= 1.0) {
            eff = newSet;
        } else {
            const double b = blend;
            eff.n = std::max(oldSet.n, newSet.n);
            for (int i = 0; i < eff.n; ++i) {
                double ore, oim, ocr, oci, nre, nim, ncr, nci;
                if (i < oldSet.n) { ore = oldSet.re[i]; oim = oldSet.im[i]; ocr = oldSet.cr[i]; oci = oldSet.ci[i]; }
                else { ore = newSet.re[i]; oim = newSet.im[i]; ocr = 0.0; oci = 0.0; }
                if (i < newSet.n) { nre = newSet.re[i]; nim = newSet.im[i]; ncr = newSet.cr[i]; nci = newSet.ci[i]; }
                else { nre = ore; nim = oim; ncr = 0.0; nci = 0.0; }
                eff.re[i] = ore + (nre - ore) * b; eff.im[i] = oim + (nim - oim) * b;
                eff.cr[i] = ocr + (ncr - ocr) * b; eff.ci[i] = oci + (nci - oci) * b;
            }
        }
        const int n = eff.n;
        for (int i = n; i < kMaxModes; ++i) pnr[i] = pni[i] = 0.0;
        M = n;
    }

    void recomputeCoefficients(double pitch, double kappa) noexcept {
        for (int i = 0; i < M; ++i) {
            const double re = eff.re[i] * (i == 0 ? kappa : 1.0), im = eff.im[i] * pitch;
            const double ex = std::exp(re * dtInt), ang = im * dtInt;
            const std::complex<double> E(ex * std::cos(ang), ex * std::sin(ang));
            const std::complex<double> G = std::complex<double>(eff.cr[i], eff.ci[i]) * (E - 1.0) / std::complex<double>(re, im);
            Er[i] = E.real(); Ei[i] = E.imag(); Gr[i] = G.real(); Gi[i] = G.imag();
        }
    }

    void control(int n) noexcept {
        const double dt = static_cast<double>(n) / fs;
        const double a = 1.0 - std::exp(-dt / tuning::kSmoothSeconds);
        const double level = breath >= 0.0f ? static_cast<double>(breath) : static_cast<double>(velocity); // blowing level 0..1
        smLevel    += a * (level - smLevel);
        smOverblow += a * (static_cast<double>(target.overblow) - smOverblow);
        smBright   += a * (static_cast<double>(target.brightness) - smBright);
        smNoise    += a * (static_cast<double>(target.breathNoise) - smNoise);
        smHardness += a * (static_cast<double>(target.reedHardness) - smHardness);
        smVibDepth += a * (static_cast<double>(target.vibratoDepth) - smVibDepth);
        smGainDb   += a * (static_cast<double>(target.outputGainDb) - smGainDb);
        setShelf();
        if (!active) return;

        vibPhase += 2.0 * kPi * static_cast<double>(target.vibratoRateHz) * dt;
        if (vibPhase > 2.0 * kPi) vibPhase -= 2.0 * kPi;
        const double vibCents = tuning::kVibratoMaxCents * smVibDepth * (0.3 + 0.7 * static_cast<double>(aftertouch)) * std::sin(vibPhase);
        const double pitch = std::exp2(static_cast<double>(bend) * 2.0 / 12.0 + vibCents / 1200.0) * static_cast<double>(target.tuningA4Hz) / 440.0;

        bool blending = false;
        if (blend < 1.0) { blend = std::min(1.0, blend + dt / blendDur); blending = true; setsDirty = true; }
        if (setsDirty) { rebuildEffective(); setsDirty = false; }

        double kappa = 1.0;
        if (forcedKappa) kappa = tuning::kKappaForced;
        else if (!registerEntry) kappa = 1.0 + (tuning::kKappaMax - 1.0) * smoothstep(tuning::kKappaStart, 1.0, smOverblow);
        if (blending || std::fabs(pitch - lastPitch) > 1e-9 || std::fabs(kappa - lastKappa) > 1e-6) {
            recomputeCoefficients(pitch, kappa);
            lastPitch = pitch; lastKappa = kappa;
        }

        // reed
        const double ft = currentBaseFreq() * pitch;
        const double fr = std::max(tuning::kReedFreqMinHz, tuning::kReedFreqRatio * ft);
        const double wr = 2.0 * kPi * fr;
        const double qr = std::max(tuning::kQrMin, tuning::kQrBase - tuning::kQrOverblowDrop * smOverblow);
        c1 = dtInt * dtInt * wr * wr;
        c2 = 0.5 * qr * wr * dtInt;
        invDen = 1.0 / (1.0 + c2);
        zeta = std::clamp(tuning::kZetaCentre + tuning::kZetaSpan * (0.5 - smHardness), tuning::kZetaMin, tuning::kZetaMax);
        reedEta = 0.01; reedKc = 100.0;

        // gamma envelope
        gammaStart = gammaEnd;
        gSm += a * (gammaTarget() - gSm);
        attackT += dt;
        double env = 1.0;
        if (inAttack) {
            env = 0.5 * (1.0 + std::tanh((attackT - 5.0 * attackTau) / attackTau));
            if (attackT >= 10.0 * attackTau) inAttack = false;
        }
        if (!gate) relGain = std::max(0.0, relGain - dt / tuning::kReleaseSeconds);
        gammaEnd = gSm * env * relGain;
        const double velTarget = breath >= 0.0f ? 1.0 : 0.5 + 0.5 * static_cast<double>(velocity);
        velGain += a * (velTarget - velGain);
    }

    // ---- audio --------------------------------------------------------------------------------
    double nextNoise() noexcept {
        noiseState ^= noiseState << 13; noiseState ^= noiseState >> 17; noiseState ^= noiseState << 5;
        return static_cast<double>(noiseState) * (2.0 / 4294967296.0) - 1.0;
    }

    void render(float* out, int n) noexcept {
        if (!active) { std::fill(out, out + n, 0.0f); return; }
        const int total = n * K;
        const double dg = (gammaEnd - gammaStart) / static_cast<double>(total);
        double g = gammaStart;
        const double gainLin = std::pow(10.0, smGainDb / 20.0);
        const double noiseGain = smNoise * tuning::kNoiseGain * (1.0 + tuning::kNoiseOverblowGain * smOverblow);
        double sumSq = 0.0;
        bool bad = false;
        double ibuf[8];
        for (int h = 0; h < n; ++h) {
            double flow = 0.0;
            for (int k = 0; k < K; ++k) {
                double p = 0.0;
                for (int i = 0; i < M; ++i) p += pnr[i];
                p *= 2.0;
                const double o = x + 1.0;
                const double mn = (o - std::sqrt(o * o + reedEta)) * 0.5;
                const double Fc = reedKc * mn * mn;
                const double xn = (c1 * (p - g + Fc - x) + 2.0 * x - xp + c2 * xp) * invDen;
                xp = x; x = xn;
                const double o2 = x + 1.0;
                const double mx = (o2 + std::sqrt(o2 * o2 + reedEta)) * 0.5;
                const double d = g - p;
                const double u = zeta * mx * sgn(d) * std::sqrt(std::sqrt(d * d + reedEta));
                for (int i = 0; i < M; ++i) {
                    const double a = pnr[i], b = pni[i];
                    pnr[i] = Er[i] * a - Ei[i] * b + Gr[i] * u;
                    pni[i] = Er[i] * b + Ei[i] * a + Gi[i] * u;
                }
                ibuf[k] = p;
                flow += std::fabs(u);
                g += dg;
            }
            int cnt = K;
            for (int s = 0; s < nStages; ++s) {
                for (int i = 0; i < cnt / 2; ++i) ibuf[i] = stages[static_cast<std::size_t>(s)].push2(ibuf[2 * i], ibuf[2 * i + 1]);
                cnt /= 2;
            }
            double v = ibuf[0];
            if (!std::isfinite(v) || std::fabs(v) > 50.0) { bad = true; v = 0.0; }
            sumSq += v * v;
            flowAvg += 0.02 * (flow / K - flowAvg);
            v += evenCoef * v * v;                       // even-harmonic content (S-016)
            v = hp.process(v);
            v = lattice.process(v);
            v = bell.process(v);
            v = shelf.process(v);
            v = tilt.process(v);
            for (auto& e : eq) v = e.process(v);
            if (noiseGain > 0.0) {
                const double nz = noiseLp.process(noiseHp.process(nextNoise()));
                v += nz * noiseGain * flowAvg;
            }
            v = dcBlock.process(v);
            v *= gainLin * velGain;
            out[h] = static_cast<float>(std::tanh(v * tuning::kOutputScale));
        }
        if (bad) {   // numerical blow-up: restart the oscillator state, keep the note
            for (int i = 0; i < kMaxModes; ++i) pnr[i] = pni[i] = 0.0;
            x = xp = 0.0;
            for (auto& s : stages) s.reset();
            hp.reset(); dcBlock.reset(); lattice.reset(); bell.reset(); shelf.reset(); tilt.reset(); for (auto& e : eq) e.reset();
        }
        const double ms = sumSq / static_cast<double>(n);
        if (!gate && ms < 1e-10) quietT += static_cast<double>(n) / fs; else quietT = 0.0;
        if (!gate && quietT >= 0.05) goIdle();
    }

    void process(float* out, int n) noexcept {
        if (out == nullptr || n <= 0) return;
        if (!prepared) { std::fill(out, out + n, 0.0f); return; }
        int done = 0;
        while (done < n) {
            const int chunk = std::min(kCtrl, n - done);
            control(chunk);
            render(out + done, chunk);
            done += chunk;
        }
    }
};

ClarinetVoice::ClarinetVoice(std::shared_ptr<const ResonatorTable> table) : impl_(std::make_unique<Impl>(std::move(table))) {
    if (!impl_->table) throw std::invalid_argument("ClarinetVoice: null resonator table");
}
ClarinetVoice::~ClarinetVoice() = default;
void ClarinetVoice::prepare(double sampleRate, int maxBlockSize) { impl_->prepare(sampleRate, maxBlockSize); }
void ClarinetVoice::reset() noexcept { if (impl_->prepared) impl_->resetAll(); }
void ClarinetVoice::setParameters(const VoiceParameters& p) noexcept { impl_->setParams(p); }
void ClarinetVoice::noteOn(int concertMidi, float velocity) noexcept { impl_->noteOn(concertMidi, velocity); }
void ClarinetVoice::noteOff(int concertMidi) noexcept { impl_->noteOff(concertMidi); }
void ClarinetVoice::allNotesOff() noexcept { impl_->nHeld = 0; impl_->gate = false; }
void ClarinetVoice::setBreath(float amount01) noexcept {
    impl_->breath = (std::isfinite(amount01) && amount01 >= 0.0f) ? std::min(amount01, 1.0f) : -1.0f;
}
void ClarinetVoice::setPitchBend(float minus1to1) noexcept { impl_->bend = sanitize(minus1to1, -1.0f, 1.0f, 0.0f); }
void ClarinetVoice::setAftertouch(float amount01) noexcept { impl_->aftertouch = sanitize(amount01, 0.0f, 1.0f, 0.0f); }
void ClarinetVoice::process(float* out, int numSamples) noexcept { impl_->process(out, numSamples); }
int ClarinetVoice::latencySamples() const noexcept { return impl_->latency; }
KeySet ClarinetVoice::currentKeys() const noexcept { return KeySet(impl_->uiKeys.load(std::memory_order_relaxed)); }
int ClarinetVoice::currentConcertNote() const noexcept { return impl_->uiNote.load(std::memory_order_relaxed); }

} // namespace clar
