# Pre-mortem round 1 (2026-10-02)

Reviewer: in-context substitution for a fresh-context Opus reviewer (A-016). Inputs: HANDOFF.md, plan/*,
tests/, research/claims.json. Prompt: "Assume the implementation was executed exactly and failed. Why?"

| # | Failure story | Category | Severity (before) |
|---|---|---|---|
| 1 | Clarion notes play the open-hole first register instead of the twelfth: the linear modal model cannot reproduce register-hole nonlinear losses (C-027). | DSP / research | Critical |
| 2 | Sound is synthetic: TinySOL metrics fail and the human hears the difference. | realism | High |
| 3 | Overblow knob only changes loudness; closed fingerings never overblow (C-033). | DSP | High |
| 4 | Altissimo cannot be tuned: lattice resonators cannot place f1 above the 1.5 kHz cutoff (C-032). | DSP | High |
| 5 | Fingering chart wrong for some notes (single source WFG). | data | High |
| 6 | Model squeaks at normal playing (reed resonance 1.5 kHz, q_r 0.4, C-022). | DSP | High |
| 7 | Legato from chalumeau to clarion lands in the wrong regime (basins, C-004). | DSP | High |
| 8 | TinySOL Pitch ID convention for the transposing clarinet misread → every comparison off by a tone. | test data | High |
| 9 | Throat/altissimo notes > 50 cents flat (reed compliance, C-013). | tuning | Medium |
| 10 | pluginval/JUCE 9 incompatibility or headless crash on Linux. | integration | Medium |
| 11 | RTF exceeds 0.05 with 24 modes × 4× oversampling. | performance | Medium |
| 12 | Implementer edits frozen tests or thresholds to pass. | process | Medium |
| 13 | Planning PAT leaks into the repository or logs. | security | Critical |
| 14 | Malformed host state/table crashes the plugin. | security | Medium |
| 15 | Output stage (tanh, EQ) creates even harmonics and erases the clarinet fingerprint. | DSP | Medium |
| 16 | T-013 "break at o = 1" too strict for low notes (κ₁ not enough). | DSP | Medium |
| 17 | Resonator generator non-deterministic → table churn, embedded data mismatch. | build | Low |
| 18 | JUCE AGPL obligations violated by distributing binaries. | licence | Low |
| 19 | LTO link exhausts memory on small machines. | build | Low |
| 20 | 4 MiB table limit or 24-mode cap too small for altissimo fits. | data | Low |

Mitigations applied after this round (in plan files):
1 → spikes C3/C4/C9; D-009 emulation (×10 mode-1 bandwidth); T-009 + T-012 + T-026 bandwidth check; decision ladder.
2 → A-012 thresholds, T-032 fingerprint test, 5 calibration rounds, G-003 blind test with human decision.
3 → D-006 κ₁ mapping (spike C5/C6); T-013 two cases; decision rules.
4 → D-009 altissimo plain-cylinder rule + f_r rule (spike C7); T-026 altissimo window; G-003 altissimo question.
5 → G-004 with `correct-chart` branch → Test Challenge Rule.
6 → q_r,base 0.7 (D-011, spike C4); T-011/T-012 at three velocities.
7 → frozen T-012 legato case; "register-change articulation" rule.
8 → D-021 (pitch from measured f0; offset recorded); T-022b requires ≥ 100 rows.
9 → D-008 per-entry calibration (scale range ±386 c covers −50/−70 c); T-010 all notes.
10 → C-019 + xvfb in CI; decision rule.
11 → D-005 power-of-two K; allowed optimisations in decision rule.
12 → FROZEN headers, manifest, T-028 in CI, HANDOFF prohibitions.
13 → PAT only in sandbox credential store outside the repo; HANDOFF forbids committing credentials; human advised to revoke.
14 → T-006, T-017 fuzz.
15 → decision rule "T-032 fails"; tanh after gain; HP ≤ 100 Hz.
16 → decision rule "T-013 break".
17 → T-031 byte-identical double generation.
18 → A-013, D-017; no distribution.
19 → ENVIRONMENT `-j1`.
20 → spike C7 shows 2–4 altissimo modes; table ~45×24 modes ≪ 4 MiB.
