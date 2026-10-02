// SPDX-License-Identifier: Apache-2.0
// Offline reference integrator (D-005): exact exponential update of the modal impedance with the flow held
// over the step, reed by central differences. A line-by-line port of research/spikes/reed_spike.py::run().
#include "clar/ColinotReference.h"
#include <cmath>
#include <complex>
#include <stdexcept>

namespace clar {

ColinotRun simulateColinot(const ResonatorParams& resonator, const ReedParams& reed, double zeta,
                           double gammaFinal, double tauG, double seconds, double sampleRate) {
    if (!(seconds > 0.0)) throw std::invalid_argument("simulateColinot: seconds must be > 0");
    if (!(tauG > 0.0)) throw std::invalid_argument("simulateColinot: tauG must be > 0");
    if (!(sampleRate >= 44100.0)) throw std::invalid_argument("simulateColinot: sampleRate must be >= 44100");
    if (resonator.modes.empty()) throw std::invalid_argument("simulateColinot: resonator has no modes");

    using cplx = std::complex<double>;
    const double dt = 1.0 / sampleRate;
    const auto N = static_cast<std::size_t>(seconds * sampleRate);
    const std::size_t M = resonator.modes.size();
    std::vector<cplx> E(M), G(M), pn(M, cplx(0.0, 0.0));
    for (std::size_t i = 0; i < M; ++i) {
        const cplx s = resonator.modes[i].pole * resonator.tuningScale;
        E[i] = std::exp(s * dt);
        G[i] = resonator.modes[i].residue * (E[i] - 1.0) / s;
    }
    const double wr = reed.omegaR;
    const double denom = 1.0 + 0.5 * reed.qR * wr * dt;
    double x = 0.0, xp = 0.0;

    ColinotRun out;
    out.sampleRate = sampleRate;
    out.pressure.resize(N);
    for (std::size_t k = 0; k < N; ++k) {
        const double t = static_cast<double>(k) * dt;
        const double g = gammaFinal / 2.0 * (1.0 + std::tanh((t - 5.0 * tauG) / tauG));
        double p = 0.0;
        for (std::size_t i = 0; i < M; ++i) p += pn[i].real();
        p *= 2.0;
        const double Fc = contactForce(x, reed);
        const double xn = (dt * dt * wr * wr * (p - g + Fc - x) + 2.0 * x - xp + 0.5 * reed.qR * wr * dt * xp) / denom;
        xp = x;
        x = xn;
        const double u = reedFlow(x, g, p, zeta, reed.eta);
        for (std::size_t i = 0; i < M; ++i) pn[i] = E[i] * pn[i] + G[i] * u;
        out.pressure[k] = static_cast<float>(p);
    }
    return out;
}

ResonatorParams colinotDSharpTable2() {
    // Colinot, Vergez, Guillemain, Doc, Acta Acustica 5 (2021) 33, Table 2 (D# fingering); real residues.
    static const double re[8] = {-17.59, -35.50, -65.30, -269.34, -70.32, -166.0, -94.49, -116.5};
    static const double im[8] = {1195.0, 2483.0, 3727.0, 4405.0, 5153.0, 6177.0, 6749.0, 7987.0};
    static const double c[8] = {176.1, 470.5, 649.4, 328.7, 541.5, 224.9, 382.2, 409.9};
    ResonatorParams r;
    for (int i = 0; i < 8; ++i) r.modes.push_back({{re[i], im[i]}, {c[i], 0.0}});
    r.tuningScale = 1.0;
    return r;
}

} // namespace clar
