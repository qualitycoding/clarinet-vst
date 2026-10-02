# Assumptions (Phase 0 intake batch, answered 2026-10-02)

The human's reply: "Add a readme.md and an apache v2 license. Use this repo
https://github.com/qualitycoding/clarinet-vst" and a PAT. Unanswered items adopt the proposed defaults
(intake items 1–16).

| ID | Assumption | Source |
|---|---|---|
| A-001 | Profiles: `software` only; `software.deploys = false`. No technical note / Zenodo record. | default 2 |
| A-002 | Instrument: B♭ soprano clarinet, Boehm system; concert D3–B♭6 (MIDI 50–94), written E3–C7 (52–96); MIDI input is concert pitch; UI also shows the written note (C-024). | defaults 3, 4 |
| A-003 | Synthesis: physical model (single-reed exciter + modal model of a virtual cylindrical clarinet bore with register hole and tone-hole lattice), no samples shipped (D-005, D-009). | default 5 |
| A-004 | "Totally realistic" = frozen objective comparison with real B♭ clarinet recordings (T-022b), the clarinet odd-harmonic fingerprint (T-032), **and** a blind listening sign-off by the human (G-003). | default 6 |
| A-005 | Reference recordings: TinySOL "Clarinet in B-flat" (CC BY 4.0), downloaded at test time, never committed or shipped (C-005). | default 7 |
| A-006 | "Levers" = the 24 player-operated keys, rings and tone holes of a Boehm clarinet (C-011, KeyId); highlight the standard fingering only (no alternates/trills in v1). | default 8 |
| A-007 | Overblown sounds = continuous **Overblow** knob (harder/brighter, then breaks to the twelfth or squeaks, D-006) + **Overblown fingering** switch (clarion notes as un-vented twelfths of chalumeau fingerings, D-007). Multiphonics/growl presets out of scope. | default 9 |
| A-008 | Monophonic legato, last-note priority; velocity + breath (CC2/CC11); pitch bend ±2 st; channel pressure → vibrato depth; portamento (glissando). | default 10 |
| A-009 | Tone controls: reed hardness, brightness, breath noise, vibrato rate/depth, tuning A4, output gain. | default 11 |
| A-010 | Performance: < 5 % of one core at 48 kHz/128 (RTF ≤ 0.05, T-016); no allocation/locks on the audio thread; latency only from the oversampling decimator (≤ 64 samples, reported to the host). | default 12 |
| A-011 | Threat model: low — offline plugin, no network at runtime, no personal data. Attack surface: host-supplied state blobs and the embedded resonator table (T-006, T-017). | default 15 |
| A-012 | Objective realism thresholds (T-022b): mean harmonic-level difference ≤ 6 dB per dynamic, ≤ 10 dB per note; centroid ratio in [0.8, 1.25] and attack-time ratio in [0.5, 2.0] for ≥ 90 % of notes; centroid increases pp<mf<ff for ≥ 90 % of pitches. Chosen by the planner (identical to the sibling saxophone plan) to reject gross timbre errors while tolerating player/instrument/recording variability; G-003 is the final arbiter. | planner (no Tier-1 standard) |
| A-013 | Licence: repository source Apache-2.0 (human instruction). JUCE used under AGPLv3 or a JUCE licence; binary distribution is out of scope, so no compliance step beyond THIRD_PARTY_NOTICES.md. | human |
| A-014 | Repository `qualitycoding/clarinet-vst`. README, LICENSE, NOTICE committed to `main` at the human's request (commit 28f0f0a); all plan artifacts on the generation branch; implementation on `impl/clarinet-v1` (D-019). | human |
| A-015 | Solo hobby maintenance: prefer readability and tests over extensibility. Independent repository: patterns copied from saxophone-vst, no shared library. | defaults 14, 16 |
| A-016 | Execution environment: single agent, no sub-agents; every "fresh-context" review ran in-context and is logged as a substitution in `.checkpoints/state.json`; tier Fable unavailable → Opus. Push uses the human-supplied PAT (never written to the repository). | human + diagnostic |
| A-017 | Formats: VST3 + Standalone on Windows/macOS/Linux, AU on macOS. Framework JUCE 9.0.3, C++20 (C-002). | default 13 |
| A-018 | CI runners: `ubuntu-24.04`, `macos-15`, `windows-2025` (GitHub-hosted). | planner |
| A-019 | MIDI: omni (all channels). Velocity-0 note-on = note-off. | planner |
| A-020 | Generative-AI imagery: none. The clarinet drawing is programmatic (JUCE Path). | planner |
| A-021 | Altissimo fingerings above G6 (G♯6–C7) are the first-listed fingerings of the WFG alternate chart because the basic chart stops at G6 (C-034); the human confirms or corrects them at G-004. | planner |
