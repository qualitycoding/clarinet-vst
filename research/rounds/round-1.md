# Research round 1 (R1 decompose + R2 breadth), 2026-10-02

Tier substitutions: single agent (Opus) performed Haiku/Sonnet breadth tasks (A-016).

- Built the question tree (research/QUESTIONS.md).
- E1–E4: re-verified sibling-plan infrastructure claims against the GitHub API today (JUCE 9.0.3 still
  latest; Catch2/nlohmann annotated tags dereferenced to the same commits; actions SHAs unchanged;
  pluginval zip checksum recomputed) → C-002, C-018, C-020.
- A1/A2: searched clarinet lattice cutoff / reed-model literature. Found Petersen et al. 2020 (S1) —
  open access, gives a complete clarinet synthesis parameter set and a designed resonator geometry →
  C-006, C-026, C-007, C-030, C-016.
- A2/A3: searched register-hole literature: Debut 2005 (geometry, twelfth tuning) and the
  Szwarcberg 2024–2026 series. **New load-bearing finding:** linear modal models do not guarantee the
  second register (C-027) — this contradicts the naive design (sax-plan style "open the vent and the model
  overblows").
- I1–I3: fetched the WFG scheme and charts; basic chart ends at G6 (C-034).
- E5: TinySOL contains "Clarinet in B-flat"; metadata header captured (C-005).

New load-bearing claims: 30. Contradictions: C-027 vs. the naive linear design (resolved in round 2).
