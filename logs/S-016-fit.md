# S-016 realism calibration log (offline, against the 126 TinySOL clarinet notes of CI run commit 0c9af03)

Method: `results.json` from the CI `realism` job stores the *recorded* per-harmonic levels (`harm_ref_db`), f0,
centroid and attack of every note. A local evaluator re-renders each note with `clar_render` (same pitch / velocity
mapping as D-015) and scores it with the same code as `tools/realism/metrics.compare` (it reproduced the CI numbers
exactly before any change). Metrics are harmonic MAD (mean |ref - syn| over harmonics 2..10, dB), centroid ratio and
dynamics ordering of our own centroids. Thresholds (A-012, frozen): harmonic MAD mean <= 6 dB per dynamic and
<= 10 dB per note; centroid ratio in [0.8, 1.25] for >= 90 %; attack ratio in [0.5, 2] for >= 90 %.

| Round | Change | pp / mf / ff harmonic MAD mean (max) | Outcome |
|---|---|---|---|
| 0 | baseline (commit 0c9af03) | 19.8 (30.6) / 13.0 (24.3) / 9.2 (17.0) | centroid in-band 93 / 36 / 24 %; dynamics ordering 100 % |
| 1 | blowing-pressure mapping (kGammaVelBase 0.45 -> 0.34, slope .3 -> .45) | 18.2 / 13.0 / 9.4 | rejected: waveform stays square-like at any pressure, spectral roll-off unchanged |
| 2 | 10-band level-interpolated EQ fitted to the residual (damped fixed-point, 8 passes) | 11.8 (27.0) / 11.9 (31.6) / 8.6 (17.8) | adopted; refinement beyond pass 3 diverged |
| 3 | even-harmonic term y = x + a x^2 (a = 0.3 at all levels) | 9.8 (24.3) / 8.9 (25.4) / 7.8 (15.6) | adopted; H2 was 12-13 dB too weak |
| 4 | mid-band lift for mf/ff to fix the centroid | centroid in-band <= 60 %, MAD +1..4 dB | rejected |
| 5 | dynamics tilt kDynTiltDb 16 -> 28 | 9.6 / 8.9 / 9.3 | adopted: dynamics ordering 88 % -> 93 % (needs >= 90 %) |

Final state: harmonic MAD mean 9.6 / 8.9 / 9.3 dB (max 25.6 / 25.4 / 17.1); centroid ratio in-band 95 / 50 / 55 %
(median 0.99 / 0.81 / 1.01); dynamics ordering 93 %. Chalumeau and throat registers are close to target (MAD 6-8 dB);
clarion 8-13 dB; altissimo 11-17 dB. None of the harmonic or centroid thresholds is met; the attack criterion is
unreachable (TEST_CHALLENGE.md). Root cause of the remaining gap: the bore model radiates a square-wave-like
internal pressure at all dynamics, whereas the recorded pp notes are nearly sinusoidal with a weak 3rd harmonic;
an output EQ can approximate the average spectrum but not the per-note shape. Per the decision rule "T-022b fails"
the work proceeds to G-003 with these metrics in the bundle (the human's listening test decides).
