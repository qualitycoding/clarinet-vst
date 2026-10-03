# Pre-mortem round 2 (2026-10-02) — re-assessment after round-1 mitigations

Reviewer: in-context substitution (A-016). New failure stories searched for after the mitigations:

| # | New/changed story | Severity |
|---|---|---|
| 21 | The emulation factor is frozen in T-031 (provenance = 10.0) but a decision rule wanted to raise it → contradiction would force a test challenge mid-implementation. | High → fixed: the T-012 ladder now changes the voice-side factor (≤ 2× extra) instead of the table. |
| 22 | T-030 register-hole test threshold (×1.15) fails for written F4 geometry (spike C11 ×1.13). | High → fixed before freeze: threshold 1.10, values recorded in the test. |
| 23 | T-013 brightness case requires FirstRegister up to o = 0.5; D-006 κ₁ starts at 0.6 → consistent. | none |
| 24 | f_r = max(1500, 2 f) changes the reed for upper clarion notes (≥ 750 Hz): untested combination with the emulation. | Medium — spike C8 used exactly this rule for written 71–84 (all twelfths). |
| 25 | T-009 depends on YIN classification of a 0.2 s half-window at 176.4 kHz; estimator range 50–2500 Hz covers 524 Hz. | Low |
| 26 | Overblown-fingering mode uses entries calibrated for R1; R2 pitch untested after calibration. | Medium — T-012 ±50 c window, spike −2..+4 c; decision rule. |
| 27 | The stub plugin build/pluginval guard (T-023) might not be verifiable in the sandbox. | Low — documented in plugin_build.log; if unverified, T-023 is not a D-018 guard and must simply pass in S-017. |

Severity of round-1 items after mitigation: 1 Medium (residual: emulation is heuristic, R-002), 2 Medium
(R-001), 3 Low, 4 Medium (R-003), 5 Medium (R-004), 6 Low, 7 Medium (R-008), 8 Low, 9 Low, 10 Medium,
11 Low, 12 Low, 13 Low (human action recommended), 14 Low, 15 Low, 16 Medium, 17–20 Low.
