# Risk register (after pre-mortem round 3)

| ID | Risk | Likelihood | Impact | Level | Mitigation / trigger | Owner |
|---|---|---|---|---|---|---|
| R-001 | Realism ceiling: objective metrics or the blind test fail after 5 calibration rounds. | medium | high | Medium | A-012 thresholds, T-032, S-016 rounds, G-003 `iterate`/`rescope` | implementer → human |
| R-002 | Register-hole emulation (heuristic, C-027) fails for some clarion notes or dynamics. | medium | high | Medium | T-009, T-012 (3 velocities), T-026; decision ladder; spikes C4/C8/C9 | implementer |
| R-003 | Altissimo approximation sounds unconvincing. | medium | medium | Medium | D-009 rule, G-003 question 4, TinySOL altissimo rows in T-022b | human |
| R-004 | Fingering fixture errors (single source C-010, C-011). | low | medium | Medium | G-004 `correct-chart` → Test Challenge Rule | human |
| R-005 | TinySOL pitch convention misread. | low | high | Low | D-021 measured-f0 rule, `pitch_offset` recorded | implementer |
| R-006 | pluginval / JUCE 9 / headless issues. | low | medium | Low | C-019, xvfb, decision rule | implementer |
| R-007 | RTF > 0.05. | low | medium | Low | decision rule optimisations | implementer |
| R-008 | Legato into the clarion lands in the wrong regime (C-004). | medium | medium | Medium | frozen T-012 legato case; articulation rule | implementer |
| R-009 | Throat/altissimo calibration does not converge. | low | medium | Low | D-008 16 iterations, γ floor rule | implementer |
| R-010 | Squeaks at normal playing. | low | medium | Low | q_r,base 0.7 (C-022), T-011/T-012 | implementer |
| R-011 | Malformed state/table crashes the host. | low | high | Low | T-006, T-017 fuzz | implementer |
| R-012 | Frozen tests modified. | low | high | Low | manifest, T-028 in CI, HANDOFF prohibitions | implementer |
| R-013 | Planning PAT exposed (it was pasted in chat). | medium | medium | Low | never written to the repo; human asked to revoke after the run; implementer uses its own credential | human |
| R-014 | Output stage destroys the odd-harmonic fingerprint. | low | medium | Low | T-032, decision rule | implementer |
| R-015 | T-013 break criterion not met. | medium | low | Medium | decision rule (κ₁ 20, earlier start) | implementer |
| R-016 | JUCE licence obligations on binaries. | low | medium | Low | no distribution (A-013, G-002 N/A) | human |
