# Traceability: success criteria → frozen tests → steps

## Success criteria
| ID | Criterion | Source |
|---|---|---|
| SC-1 | VST3 + Standalone (all OSes) and AU (macOS) instrument plugin with the D-013 parameters, passing pluginval strictness 10. | A-017, D-013 |
| SC-2 | The UI draws a Boehm clarinet and highlights exactly the levers of the fingering in use, within one audio block of the note change. | request, A-006, D-004, D-016 |
| SC-3 | Every note D3–B♭6 sounds within ±10 cents of 12-TET (A4 adjustable), independent of sample rate. | A-002, D-008 |
| SC-4 | "Totally realistic": frozen objective comparison with TinySOL clarinet recordings, the clarinet odd-harmonic fingerprint, and the human's blind listening sign-off. | A-004, A-012, G-003 |
| SC-5 | Overblown sounds: Overblow knob brightens, then breaks notes to the twelfth/squeak; Overblown-fingering switch plays clarion notes as un-vented twelfths; register key produces the twelfth for clarion notes. | A-007, D-006, D-007, D-009 |
| SC-6 | Expressive mono legato with velocity/breath, pitch bend, aftertouch vibrato, portamento. | A-008, D-011 |
| SC-7 | Real-time safe and efficient: no allocation in the audio path; RTF ≤ 0.05 at 48 kHz/128. | A-010, D-010 |
| SC-8 | Host state round-trips all parameters; garbage state is ignored. | D-013 |
| SC-9 | Robust under random abuse; deterministic; works at 22.05–192 kHz and 1–4096-sample blocks. | A-011, D-010 |
| SC-10 | Supply chain pinned; licences and notices present. | D-002, D-017, A-013 |

## Tests
Type: U unit, I integration, O operational, P performance, S security. "Red" = verified failing against stubs
on 2026-10-02 (D-018 guards pass by design). Fail mode = how a failure would surface.

| Test | File | Type | Enforces | Claims / decisions | Red | Fail mode | Step |
|---|---|---|---|---|---|---|---|
| T-001 | tests/core/test_pitch.cpp | U | SC-3, SC-2 | C-024 | fail | assertion | S-003 |
| T-002 | tests/core/test_keys.cpp | U | SC-2 | C-011 | fail | assertion | S-003 |
| T-003 | tests/core/test_fingering.cpp | U | SC-2 | C-010, D-004 | fail | exception | S-003 |
| T-004 | tests/core/test_fingering.cpp | U | SC-2, SC-5 | C-012, C-025, D-004 | fail | exception | S-003 |
| T-005 | tests/core/test_fingering.cpp | U | SC-5, SC-2 | D-007 | fail | assertion | S-003 |
| T-006 | tests/core/test_resonator_table.cpp | U, S | SC-9 | D-009 | fail | exception type | S-007 |
| T-007 | tests/core/test_reed_model.cpp | U | SC-4 | C-006, C-026 | fail | assertion (NaN) | S-005 |
| T-008 | tests/core/test_reference_integrator.cpp | I | SC-4 | C-006, C-008, D-005 | fail | exception | S-005 |
| T-009 | tests/core/test_reference_integrator.cpp | I | SC-5 | C-027, C-035, D-009 | fail | exception | S-005 |
| T-010 | tests/core/test_voice_pitch.cpp | I | SC-3 | C-013, C-029, D-008 | fail | exception | S-009 |
| T-011 | tests/core/test_voice_registers.cpp | I | SC-5, SC-4 | C-022, D-011 | fail | exception | S-011 |
| T-012 | tests/core/test_voice_registers.cpp | I | SC-5 | C-004, C-027, C-033, C-035, D-007, D-009 | fail | exception | S-011 |
| T-013 | tests/core/test_voice_registers.cpp | I | SC-5 | C-033, D-006 | fail | exception | S-011 |
| T-014 | tests/core/test_voice_robustness.cpp | O | SC-9 | D-010 | fail | exception | S-008 |
| T-015 | tests/core/test_alloc.cpp | P | SC-7 | D-010 | fail | exception | S-008 |
| T-016 | tests/core/test_perf.cpp | P | SC-7 | A-010 | skip (Debug) / fail (Release) | exception | S-017 |
| T-017 | tests/core/test_state.cpp | U, S | SC-8, SC-9 | D-013 | fail | exception | S-012 |
| T-018 | tests/core/test_key_layout.cpp | U | SC-2 | D-016 | fail | assertion | S-003 |
| T-019 | tests/core/test_midi_behaviour.cpp | I | SC-2, SC-6 | D-007, D-011 | fail | exception | S-010 |
| T-020 | tests/core/test_voice_pitch.cpp | I | SC-3 | D-005 | fail | exception | S-008 |
| T-021 | tests/core/test_voice_robustness.cpp | O | SC-9 | D-010 | fail | exception | S-008 |
| T-022a | tests/python/test_realism_metrics.py | U | SC-4 | D-015 | fail | NotImplementedError | S-004 |
| T-022b | tests/python/test_realism_vs_tinysol.py | I | SC-4 | A-012, C-005, D-015, D-021 | fail | assertion (env/NotImplemented) | S-016 |
| T-023 | tests/scripts/run_pluginval.sh | O, I | SC-1 | C-019 | guard (D-018) | exit code | S-017 |
| T-024 | tests/python/test_supply_chain.py | S | SC-10 | D-002, D-017, D-020 | a/b guard; c/d fail | assertion | S-002 |
| T-025 | tests/core/test_voice_robustness.cpp | O | SC-9 | D-010 | fail | exception | S-008 |
| T-026 | tests/core/test_resonator_table.cpp | I | SC-3, SC-5 | C-013, C-025, C-027, D-008, D-009 | fail | exception | S-007, S-009 |
| T-027 | tests/core/test_analysis.cpp | U | SC-3, SC-4, SC-5 | D-015 | fail | assertion | S-004 |
| T-028 | tests/scripts/verify_freeze.sh | S | integrity | Rule 2E | guard (D-018) | exit code | every step |
| T-029 | tests/plugin/test_plugin.cpp | I | SC-1, SC-2, SC-8 | D-013, D-016 | param-ID case guard; others fail | assertion | S-013, S-014 |
| T-030 | tests/python/test_resonator_tmm.py | U | SC-4, SC-5 | C-028, C-030, C-031, D-009 | fail | NotImplementedError | S-006 |
| T-031 | tests/python/test_resonator_tmm.py | I | SC-3, SC-5 | D-009 | fail | CalledProcessError | S-007 |
| T-032 | tests/core/test_timbre.cpp | I | SC-4 | C-025, C-030, D-014 | fail | exception | S-016 |

Coverage check: every SC has ≥ 1 test; SC-4 additionally gated by G-003, SC-2 by G-004. Test types covered:
unit, integration, operational, performance, security (Rule 2B.1).

## Step ↔ evidence
S-002 T-024 · S-003 T-001..T-005, T-018 · S-004 T-022a, T-027 · S-005 T-007, T-008, T-009 · S-006 T-030 ·
S-007 T-006, T-031, T-026 (structure) · S-008 T-014, T-015, T-020, T-021, T-025 · S-009 T-010, T-026 ·
S-010 T-019 · S-011 T-011, T-012, T-013 · S-012 T-017 · S-013/S-014 T-029 · S-016 T-022b, T-032 ·
S-017 T-016, T-023, T-029 (3 OSes) · S-018 all.
