# Research round 2 (R3 synthesis + R4 depth + R6 spikes), 2026-10-02

- Synthesis/gap analysis: the design depends on (i) the TMM reproducing a clarinet-like lattice,
  (ii) a working clarion mechanism despite C-027, (iii) an overblow mechanism on closed fingerings,
  (iv) altissimo above the cutoff. All four were single-source or untested → spikes.
- Spike C1 (tmm_spike.py): plain TMM reproduces Petersen Table 1 f1 = 185 Hz (184.8 Hz).
- Spike C2 (reed_spike.py): Petersen reed + virtual clarinet (a 7.5 mm, lattice cell scaled from
  Table 1 so eq. (1) keeps fc = 1.5 kHz, register hole per Debut) plays written G3 at 174.3–175.0 Hz with
  H2 at −46 dB vs H3 −10 dB (odd fingerprint). Register hole open, linear: twelfth in 1/12 runs.
- Spike C3 (clarion_map.py) + diagnostic: "other" outcomes were squeaks near 1.23 kHz / open-hole R1.
- Spike C4 (reed_map.py): mode-1 damping ×10 and q_r ≥ 0.7 (or f_r 2.2 kHz) → 6/6 twelfths; closed hole
  always R1 → C-022, C-035.
- Spike C5 (overblow_map.py): closed hole never overblows by gamma/tau/q_r/zeta alone; mode-1 suppression
  does → C-033.
- Spike C6/C7: lattice altissimo impossible above cutoff; plain short cylinder with f_r = 2 f works
  (−62..−73 c pre-calibration) → C-032; throat notes −50 c pre-calibration → C-013.
- Spike C8: upper clarion +1..+29 c pre-calibration → C-029.
- Depth: Debut 2005 sections 3.3/4.3 read for register-hole parameters (C-028); Szwarcberg 2025 full text
  read (C-004, C-009, C-031).

Confidence changes: C-027 single-source → verified (spikes); C-030 corroborated → verified (spike C10 later).
New questions: A3.2 legato across the break (C-004) → frozen T-012 legato case + decision rule.
