// SPDX-License-Identifier: Apache-2.0
// Offline signal analysis (never on the audio thread): YIN f0, regime classification, harmonic levels,
// spectral centroid. Self-contained radix-2 FFT, no third-party code.
#include "clar/Analysis.h"
#include <algorithm>
#include <cmath>
#include <complex>
#include <limits>
#include <numbers>
#include <stdexcept>

namespace clar {
namespace {

using cplx = std::complex<double>;

std::size_t nextPow2(std::size_t n) {
    std::size_t p = 1;
    while (p < n) p <<= 1;
    return p;
}

void fft(std::vector<cplx>& a, bool inverse) {
    const std::size_t n = a.size();
    for (std::size_t i = 1, j = 0; i < n; ++i) {
        std::size_t bitm = n >> 1;
        for (; j & bitm; bitm >>= 1) j ^= bitm;
        j ^= bitm;
        if (i < j) std::swap(a[i], a[j]);
    }
    for (std::size_t len = 2; len <= n; len <<= 1) {
        const double ang = 2.0 * std::numbers::pi / static_cast<double>(len) * (inverse ? 1.0 : -1.0);
        const cplx wl(std::cos(ang), std::sin(ang));
        for (std::size_t i = 0; i < n; i += len) {
            cplx w(1.0, 0.0);
            for (std::size_t k = 0; k < len / 2; ++k) {
                const cplx u = a[i + k];
                const cplx v = a[i + k + len / 2] * w;
                a[i + k] = u + v;
                a[i + k + len / 2] = u - v;
                w *= wl;
            }
        }
    }
    if (inverse)
        for (auto& v : a) v /= static_cast<double>(n);
}

double rms(std::span<const float> x) {
    if (x.empty()) return 0.0;
    double s = 0.0;
    for (const float v : x) s += static_cast<double>(v) * static_cast<double>(v);
    return std::sqrt(s / static_cast<double>(x.size()));
}

/// Hann-windowed power spectrum of x, zero-padded to `padFactor`*length rounded up to a power of two.
std::vector<double> hannPower(std::span<const float> x, std::size_t padFactor, std::size_t& fftSize) {
    const std::size_t n = x.size();
    fftSize = nextPow2(n * padFactor);
    std::vector<cplx> a(fftSize, cplx(0.0, 0.0));
    double mean = 0.0;
    for (const float v : x) mean += static_cast<double>(v);
    mean /= static_cast<double>(n);
    for (std::size_t i = 0; i < n; ++i) {
        const double w = 0.5 - 0.5 * std::cos(2.0 * std::numbers::pi * static_cast<double>(i) / static_cast<double>(n));
        a[i] = (static_cast<double>(x[i]) - mean) * w;
    }
    fft(a, false);
    std::vector<double> p(fftSize / 2 + 1);
    for (std::size_t k = 0; k < p.size(); ++k) p[k] = std::norm(a[k]);
    return p;
}

} // namespace

double estimateF0(std::span<const float> xin, double sampleRate) noexcept {
    try {
        const std::size_t cap = static_cast<std::size_t>(sampleRate); // last 1.0 s
        std::span<const float> x = xin.size() > cap ? xin.subspan(xin.size() - cap) : xin;
        if (x.size() < 64 || rms(x) < 1e-4) return 0.0;
        const std::size_t tauMin = std::max<std::size_t>(2, static_cast<std::size_t>(sampleRate / 2500.0));
        const std::size_t tauMax = std::min<std::size_t>(static_cast<std::size_t>(sampleRate / 50.0) + 1, x.size() / 2);
        if (tauMax <= tauMin + 2) return 0.0;
        const std::size_t W = x.size() - tauMax;
        // difference function d(tau) = sum_j (x_j - x_{j+tau})^2 via FFT cross-correlation (j < W)
        const std::size_t M = nextPow2(W + tauMax + 1);
        std::vector<cplx> A(M, cplx(0, 0)), B(M, cplx(0, 0));
        for (std::size_t j = 0; j < W; ++j) A[j] = static_cast<double>(x[j]);
        for (std::size_t j = 0; j < W + tauMax; ++j) B[j] = static_cast<double>(x[j]);
        fft(A, false);
        fft(B, false);
        for (std::size_t k = 0; k < M; ++k) A[k] = std::conj(A[k]) * B[k];
        fft(A, true);
        std::vector<double> csum(x.size() + 1, 0.0); // cumulative energy
        for (std::size_t j = 0; j < x.size(); ++j)
            csum[j + 1] = csum[j] + static_cast<double>(x[j]) * static_cast<double>(x[j]);
        std::vector<double> d(tauMax + 1, 0.0), dn(tauMax + 1, 1.0);
        for (std::size_t t = 1; t <= tauMax; ++t) d[t] = csum[W] + (csum[W + t] - csum[t]) - 2.0 * A[t].real();
        double run = 0.0;
        for (std::size_t t = 1; t <= tauMax; ++t) {
            run += d[t];
            dn[t] = run > 0.0 ? d[t] * static_cast<double>(t) / run : 1.0;
        }
        std::size_t best = 0;
        for (std::size_t t = tauMin; t < tauMax; ++t) {
            if (dn[t] < 0.1) {
                while (t + 1 < tauMax && dn[t + 1] < dn[t]) ++t;
                best = t;
                break;
            }
        }
        if (best == 0) {
            double m = std::numeric_limits<double>::max();
            for (std::size_t t = tauMin; t < tauMax; ++t)
                if (dn[t] < m) { m = dn[t]; best = t; }
            if (m > 0.3) return 0.0; // no periodicity
        }
        double tau = static_cast<double>(best);
        if (best > 1 && best + 1 <= tauMax) {
            const double a = d[best - 1], b = d[best], c = d[best + 1];
            const double den = a - 2.0 * b + c;
            if (den > 0.0) tau += 0.5 * (a - c) / den;
        }
        return sampleRate / tau;
    } catch (...) {
        return -1.0;
    }
}

Regime classifyRegime(std::span<const float> x, double sampleRate, double expectedR1Hz) noexcept {
    if (x.empty()) return Regime::Silent;
    const std::span<const float> tail = x.subspan(x.size() / 2);
    if (rms(tail) < 1e-4) return Regime::Silent;
    const double f0 = estimateF0(tail, sampleRate);
    if (!(f0 > 0.0) || !(expectedR1Hz > 0.0)) return Regime::Other;
    const double r = f0 / expectedR1Hz;
    if (r >= 0.94 && r <= 1.06) return Regime::FirstRegister;
    if (r >= 2.82 && r <= 3.18) return Regime::Twelfth;
    return Regime::Other;
}

std::vector<double> harmonicLevelsDb(std::span<const float> x, double sampleRate, double f0, int n) {
    if (!(f0 > 0.0) || n < 1) throw std::invalid_argument("harmonicLevelsDb: f0 must be > 0 and n >= 1");
    std::size_t fftSize = 0;
    const std::vector<double> p = hannPower(x, 4, fftSize);
    const double binHz = sampleRate / static_cast<double>(fftSize);
    std::vector<double> amp(static_cast<std::size_t>(n), 0.0);
    for (int h = 1; h <= n; ++h) {
        const double lo = static_cast<double>(h) * f0 * 0.97, hi = static_cast<double>(h) * f0 * 1.03;
        const auto k0 = static_cast<std::size_t>(std::max(0.0, std::floor(lo / binHz)));
        const auto k1 = std::min(p.size() - 1, static_cast<std::size_t>(std::ceil(hi / binHz)));
        double m = 0.0;
        for (std::size_t k = k0; k <= k1; ++k) m = std::max(m, p[k]);
        amp[static_cast<std::size_t>(h - 1)] = std::sqrt(m);
    }
    const double top = *std::max_element(amp.begin(), amp.end());
    std::vector<double> out(amp.size(), -300.0);
    if (top <= 0.0) return out;
    for (std::size_t i = 0; i < amp.size(); ++i)
        if (amp[i] > 0.0) out[i] = std::max(-300.0, 20.0 * std::log10(amp[i] / top));
    return out;
}

double spectralCentroidHz(std::span<const float> x, double sampleRate) noexcept {
    try {
        if (x.size() < 16) return -1.0;
        std::size_t fftSize = 0;
        const std::vector<double> p = hannPower(x, 1, fftSize);
        const double binHz = sampleRate / static_cast<double>(fftSize);
        double num = 0.0, den = 0.0;
        for (std::size_t k = 0; k < p.size(); ++k) {
            const double f = static_cast<double>(k) * binHz;
            if (f < 20.0) continue;
            num += f * p[k];
            den += p[k];
        }
        return den > 0.0 ? num / den : -1.0;
    } catch (...) {
        return -1.0;
    }
}

double cents(double a, double b) noexcept {
    if (!(a > 0.0) || !(b > 0.0)) return std::numeric_limits<double>::quiet_NaN();
    return 1200.0 * std::log2(a / b);
}

} // namespace clar
