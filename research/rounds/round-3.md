# Research round 3 (R5 adversarial pass + R6 frozen-procedure replays), 2026-10-02

Fresh-context adversarial agent unavailable; performed in-context as a substitution (A-016). Each
load-bearing claim was attacked:

- C-027/C-035 (emulation): "the emulation only works for the one geometry tested" → spike C8 tested
  written 71/74/77/80/84 (all twelfths, +1..+29 c); spike C9 replayed the frozen T-009 procedure with the
  fixture rounded to 6 significant digits (linear 3/6, emulated 6/6). Residual risk R-002.
- C-030/C-031 (TMM): "the loss model is wrong" → spike C10 reproduces all three Petersen resonators
  (f1 within 0.2 %, cutoffs within 11 %).
- C-025 (odd series): "lattice destroys the 3:1 ratio for short fingerings" → spike C11: 2.981 at
  written F4, 2.916 at throat Bb (excluded from the T-026 ratio check).
- C-010 (fingerings): single source (WFG, Tier 3); no Tier-1 standard exists for "standard fingering".
  Searched for a second independent chart: only derivative/low-quality pages found. Remains
  single-source → R-004, mitigated by G-004 human check.
- C-011 (lever set): WFG lists LH F/C on Boehm models; some instruments lack it → drawn anyway (only used
  if a fingering needs it; the fixture uses LH F only via alternate forms not chosen). Single-source,
  low impact → G-004.
- C-021 (reed resonance ≈ cutoff): single source, not load-bearing for a test; informs the choice of
  f_r only.
- C-005 (TinySOL pitch convention): whether "Pitch ID" is concert or written for the clarinet is not
  stated → D-021 determines it from the measured f0 of each file (robust either way).
- Known breaking changes: JUCE 9.0.3 vs pluginval 1.0.4 was verified in the sibling plan; re-verified
  here on this plan's stub (research/spikes/plugin_build.log, if the build completes).

Termination: round 3 produced no new load-bearing claims, no downgrades and no unresolved
contradictions (saturation). Load-bearing claims not reaching corroborated/verified: C-010, C-011
(single-source) → risk register R-004.
