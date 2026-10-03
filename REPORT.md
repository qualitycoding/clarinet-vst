# REPORT — clarinet-vst v1 (branch `impl/clarinet-v1`)

Status: **implementation complete; awaiting human decisions** (G-003, and approval of two frozen-test corrections).
Nothing has been merged to `main`, released or published (merging is the human's decision, D-019).

## What was built
A B♭ soprano clarinet instrument plugin (VST3 + Standalone on Windows/macOS/Linux, AU on macOS), C++20 on JUCE 9.0.3:
- **Sound:** physical model. Single-reed exciter (Petersen 2020 / Colinot 2021 equations) coupled to a modal resonator
  of a virtual clarinet bore generated offline (transfer-matrix model with tone-hole lattice and register hole,
  `tools/resonator`), 45 resonators covering written E3–C7, embedded in the binary. No samples are shipped.
- **UI:** a drawn Boehm clarinet; the keys, rings and holes of the fingering in use are highlighted (24 levers, the thumb
  hole and register key in a rear-view inset), with a caption (fingering note, concert note, register).
- **Overblowing:** an *Overblow* knob (brighter, then the note breaks to the twelfth or squeaks) and an *Overblown
  fingering* switch (clarion notes played as un-vented twelfths of the chalumeau fingering, shown without the register key).
- **Playing:** monophonic legato, velocity or breath (CC2/CC11), pitch bend ±2 st, aftertouch vibrato, portamento.
- **Tools:** `clar_render` (single notes to WAV), `clar_demo` (listening demos), `clar_calibrate` (per-note tuning),
  `clar_ui_snapshots` (screenshots), Python realism tools against the TinySOL recordings.

## Success criteria
| ID | Criterion | Status | Evidence |
|---|---|---|---|
| SC-1 | VST3/Standalone/AU, parameters, pluginval strictness 10 | **met** | CI `plugin` job green on Linux, macOS (also `auval` PASS), Windows; T-023, T-029 |
| SC-2 | UI highlights exactly the levers of the fingering within one block | **met** (human check of the chart pending = G-004, treated as `proceed`) | T-003, T-004, T-005, T-018, T-019, T-029; 60 screenshots in `gates/G-004/` |
| SC-3 | every note within ±10 cents, sample-rate independent | **met** | T-010 (worst residual 1.4 cents), T-020 |
| SC-4 | "totally realistic" | **not met objectively; awaiting G-003** | T-022b fails; see below |
| SC-5 | overblow behaviour | **met** | T-009, T-011, T-012, T-013 |
| SC-6 | expressive mono legato | **met** | T-019 (+ checks of bend, vibrato, glide) |
| SC-7 | real-time safe, RTF ≤ 0.05 | **met** | T-015, T-016 (also green on 3 OSes in CI) |
| SC-8 | state round-trip, garbage ignored | **met** | T-017, T-029 |
| SC-9 | robustness, determinism | **met** | T-014, T-021, T-025 |
| SC-10 | supply chain pinned, licences | **met** | T-024 |

## Tests (frozen, 28 files, hashes in `tests/FROZEN_MANIFEST.sha256`; `verify_freeze.sh` = freeze OK)
| Result | Tests |
|---|---|
| pass | T-002 … T-021 (except T-001), T-022a, T-023, T-024, T-025 … T-032, T-018 |
| **fail — frozen constant is wrong** | **T-001**: one assertion expects 1872.4721 Hz for MIDI 94 at A4 = 442; correct is 1873.1307508272341. Everything else in T-001 passes. `TEST_CHALLENGE.md` |
| **fail — realism** | **T-022b** (CI, 126 real notes). Attack criterion infeasible (`TEST_CHALLENGE.md`); harmonic and centroid thresholds not met |

Local run (Release): unit 17/18 cases (T-001), integration, operational, alloc, perf all pass; Python 24/24 (excluding
T-022b, which needs Zenodo and runs in CI). CI per commit: `freeze`, `python`, `plugin` ×3 green; `core` ×3 red only
because of T-001; `realism` red because of T-022b.

## Realism (SC-4) in numbers — real TinySOL recordings, 126 notes
| | harmonic error mean (≤ 6 dB) | worst note (≤ 10 dB) | brightness in ±25 % (≥ 90 %) |
|---|---|---|---|
| pp | 9.7 dB | 26.6 dB | 95 % |
| mf | 8.9 dB | 25.4 dB | 50 % |
| ff | 9.3 dB | 16.9 dB | 55 % |

Louder is brighter for 93 % of pitches (≥ 90 % required: met). The odd-harmonic fingerprint matches (H3 − H2 for low
notes: ours 12.7 / 21.4 / 20.8 dB, real 15.1 / 17.0 / 28.6 dB). Five calibration rounds (`logs/S-016-fit.md`) took the
harmonic error from 19.8 / 13.0 / 9.2 to the table above. Remaining gap: the bore model radiates a square-wave-like
pressure at every dynamic, while recorded soft notes are nearly sinusoidal; an output EQ fixes the average spectrum, not
the per-note shape. Weakest registers: clarion and altissimo (simplified resonators). Whether it sounds realistic is
the human's call at **G-003** (`GATE-G-003.md`, bundle on branch `ci-g003`).

## Gates
- G-004 (fingering chart / UI): human replied "proceed" → recorded as `proceed`.
- G-003 (realism sign-off): **pending**.
- G-001, G-002: not applicable (no headline result; nothing released or published).

## Deviations and risks
All deviations are in `DEVIATIONS.md`. Notable: Overblow tilt shelf and lip-damping retune (S-011), dynamics
brightening, radiation EQ and even-harmonic term (S-016), `sha256sum` path workaround for the frozen pluginval
script on Windows, TinySOL result sidecar file. Open risks: realism ceiling (R-001), altissimo approximation (R-003),
fingering chart is single-sourced (R-004), legato into the clarion not covered beyond three test transitions (R-008).

## Not done / not verified
- No test in a real DAW host (only pluginval, auval and the headless plugin tests).
- G-003 listening judgement; the T-001 and T-022b corrections need approval before CI can go fully green.
- Merging to `main`, tagging, releasing and distributing binaries (JUCE AGPLv3/commercial licence applies, see
  `THIRD_PARTY_NOTICES.md`).
