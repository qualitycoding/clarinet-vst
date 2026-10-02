<!-- STATUS: planning complete 2026-10-02 UTC. Branch: gen-20261002T075147Z-clarinet-vst-plan. Profile: software (deploys=false). Steps S-001..S-018; gates G-004, G-003 (G-001/G-002 N/A). Frozen tests: 28 files in tests/FROZEN_MANIFEST.sha256, red verified except D-018 guards. Open risks (Medium): R-001, R-002, R-003, R-004, R-008, R-015. Substitutions: single agent, in-context reviews (A-016). -->
# Execution plan — clarinet-vst v1

Conventions: run all commands from the repository root on branch `impl/clarinet-v1` (D-019). "Build" means
`cmake --build build -j1` (or `-j` on machines with > 4 GB RAM). After each step: commit `S-0xx: <title>`,
update `.checkpoints/impl-state.json` (`{"step":"S-0xx","status":"done","commit":"<sha>","utc":"…"}`), push.
Every step is idempotent: re-running it after partial completion is safe; completion is detected by its
"Done when" checks. Run `bash tests/scripts/verify_freeze.sh` before and after every step.

### S-001 Environment and baseline
- Tier: Sonnet
- Profile: software
- Depends on: none
- Inputs: generation branch head; plan/ENVIRONMENT.md
- Actions:
  1. `git checkout -b impl/clarinet-v1 origin/<generation branch>` (or check it out if it exists).
  2. Install system packages and the Python venv exactly as in plan/ENVIRONMENT.md.
  3. `bash tests/scripts/verify_freeze.sh`.
  4. `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release` and build.
  5. `ctest --test-dir build --output-on-failure || true`; `python -m pytest tests/python || true`; save both outputs to `logs/S-001-red.txt`.
- Outputs: build/, .venv/, logs/S-001-red.txt
- Evidence produced: none (baseline)
- Done when: `freeze OK`; build succeeds (plugin included); every test except the D-018 guards fails.
- Checkpoint: step, commit, red counts.
- On failure: missing packages → install and retry; stub build failure → `BLOCKED.md` (stubs were verified to compile).
- Gate: none
- Relevant decisions/claims: D-001, D-002, D-012, D-018, C-002, C-023

### S-002 CI workflow and third-party notices
- Tier: Sonnet
- Profile: software
- Depends on: S-001
- Inputs: D-017, D-020
- Actions:
  1. Create `.github/workflows/ci.yml` with the five jobs of D-020, `on: [push, pull_request]`, `permissions: contents: read`, actions pinned to the D-020 SHAs.
  2. Create `THIRD_PARTY_NOTICES.md` with the D-017 entries (name, licence, URL, use, shipped yes/no).
  3. Push; open the Actions run.
- Outputs: .github/workflows/ci.yml, THIRD_PARTY_NOTICES.md
- Evidence produced: T-024 (all four cases)
- Done when: `python -m pytest tests/python/test_supply_chain.py` passes; CI `freeze` job green.
- Checkpoint: step, commit, CI run URL.
- On failure: YAML error → fix; never unpin actions.
- Gate: none
- Relevant decisions/claims: D-017, D-020, C-020

### S-003 Pitch, levers, fingering, key layout
- Tier: Sonnet
- Profile: software
- Depends on: S-001
- Inputs: core/include/clar/{Pitch,Keys,Fingering,KeyLayout}.h; tests/fixtures/clarinet_standard_fingerings.txt
- Actions:
  1. Implement `Pitch.cpp`, `Keys.cpp` (names exactly as in the Keys.h comments), `Fingering.cpp` (a static table of 45 entries transcribed from the fixture — do not read the fixture at runtime) and `resolveNote` per D-007.
  2. Implement `KeyLayout.cpp` per D-016 in normalised coordinates: body centred at x ≈ 0.45; LH1/LH2/LH3 then RH1/RH2/RH3 as ring holes top-to-bottom; A key above-left of LH1, side G♯ left of LH1; LH Eb sliver right of LH2; LH pinky cluster (C♯, E, F, F♯) below-left of LH3; side keys 1–4 stacked right of RH1–RH2; RH B sliver right of RH2; RH pinky cluster (E, F, F♯, G♯) below-right of RH3; rear inset at x ≈ 0.1 with thumb ring hole and register key above it. No shared centres.
  3. Build; run `build/tests/clar_unit_tests "[T-001],[T-002],[T-003],[T-004],[T-005],[T-018]"`.
- Outputs: core/src/{Pitch,Keys,Fingering,KeyLayout}.cpp
- Evidence produced: T-001, T-002, T-003, T-004, T-005, T-018
- Done when: those tags pass.
- Checkpoint: step, commit, passing tags.
- On failure: fixture mismatch → fix the table (the fixture is authoritative); fixture believed wrong → `TEST_CHALLENGE.md`.
- Gate: none
- Relevant decisions/claims: D-004, D-007, D-016, C-010, C-011, C-012, C-024

### S-004 Analysis utilities (C++ and Python)
- Tier: Sonnet
- Profile: software
- Depends on: S-001
- Inputs: core/include/clar/Analysis.h; tools/realism/metrics.py
- Actions:
  1. Implement YIN (de Cheveigné & Kawahara 2002: difference function, cumulative-mean normalisation, threshold 0.1, parabolic interpolation, search 50–2500 Hz, frame = whole input capped at the last 1.0 s), `classifyRegime` exactly per header, `harmonicLevelsDb` (Hann, zero-pad to next pow2 ≥ 4×len, peak within ±3 % of h·f0), `spectralCentroidHz` (power spectrum 20 Hz–fs/2), `cents`. Self-written radix-2 FFT (no new dependency).
  2. Implement the same metrics in `tools/realism/metrics.py` (numpy/scipy) incl. `attack_time_s`, `steady_segment`, `compare` per docstrings.
  3. Run `build/tests/clar_unit_tests "[T-027]"` and `python -m pytest tests/python/test_realism_metrics.py`.
- Outputs: core/src/Analysis.cpp, tools/realism/metrics.py
- Evidence produced: T-027, T-022a
- Done when: both pass.
- Checkpoint: step, commit.
- On failure: f0 error > 2 cents → check interpolation and the 1.0 s cap.
- Gate: none
- Relevant decisions/claims: D-015

### S-005 Reed model and reference integrator
- Tier: Opus
- Profile: software
- Depends on: S-004
- Inputs: ReedModel.h, ColinotReference.h, research/spikes/reed_spike.py (`run()`), C-006, C-026, Colinot 2021 Table 2
- Actions:
  1. Implement `clarinetReed()` (C-026) and eqs. (5)–(10) in `ReedModel.cpp`.
  2. Implement `simulateColinot` as a line-by-line port of `run()` in `research/spikes/reed_spike.py` (complex residues; γ(t) = γ_f/2·(1 + tanh((t − 5τ)/τ))) in double precision; `colinotDSharpTable2()` returns Colinot 2021 Table 2 (8 modes, real residues: s = −17.59+1195j, −35.50+2483j, −65.30+3727j, −269.34+4405j, −70.32+5153j, −166.0+6177j, −94.49+6749j, −116.5+7987j; C = 176.1, 470.5, 649.4, 328.7, 541.5, 224.9, 382.2, 409.9). Argument checks per header.
  3. Run `build/tests/clar_unit_tests "[T-007]"` and `build/tests/clar_integration_tests "[T-008],[T-009]"`.
- Outputs: core/src/ReedModel.cpp, core/src/ColinotReference.cpp
- Evidence produced: T-007, T-008, T-009
- Done when: those tags pass.
- Checkpoint: step, commit, measured f0 and twelfth counts.
- On failure: decision rules "T-008" and "T-009".
- Gate: none
- Relevant decisions/claims: D-005, C-006, C-008, C-026, C-027, C-035

### S-006 Transfer-matrix model and modal fit (Python)
- Tier: Opus
- Profile: software
- Depends on: S-001
- Inputs: tools/resonator/tmm.py; research/spikes/tmm_spike.py, reed_spike.py (reference implementations); D-009
- Actions:
  1. Implement the TMM per the module docstring (equations of `research/spikes/tmm_spike.py`; `lossless=True` → Γ = s/c; `radiation=False` → Z_R = 0), `cylinder_only_impedance`, `petersen_resonator` (C-030), `clarinet_geometry` per D-009 (bisection to 1e-6 m on the first peak found by `peak_frequencies` over 40–2500 Hz with 0.05 Hz grid), `peak_frequencies`.
  2. `modal_fit`: poles from peaks (frequency, half-power bandwidth), complex residues by linear least squares, then `scipy.optimize.least_squares` refinement of all parameters on complex Z with relative weighting; enforce Re(s) < 0; sort by Im.
  3. `python -m pytest tests/python/test_resonator_tmm.py -k "not t031"`.
- Outputs: tools/resonator/tmm.py
- Evidence produced: T-030
- Done when: the non-T-031 tests pass.
- Checkpoint: step, commit.
- On failure: decision rule "T-030"; modal-fit test → widen the initialisation span, never the tolerances.
- Gate: none
- Relevant decisions/claims: D-009, C-028, C-030, C-031

### S-007 Resonator table: generation, parsing, embedding
- Tier: Sonnet
- Profile: software
- Depends on: S-003, S-006
- Inputs: D-009; ResonatorTable.h
- Actions:
  1. Implement `generate_table.py`: 45 entries per D-009 (fit range, ≤ 24 modes, register-entry emulation `Re(s_1)×NL_MODE1_BW_FACTOR`), deterministic (fixed grids, no randomness, `json.dumps(…, indent=1, sort_keys=True)`, floats to 6 significant digits, `provenance.git = git rev-parse HEAD` or "unknown", `provenance.nl_mode1_bw_factor = 10.0`).
  2. `python -m tools.resonator.generate_table --out data/clarinet_resonators.json`.
  3. Implement `ResonatorTable::fromJson/lookup/size` with nlohmann/json and the D-009 validation (size check first; wrap every exception as `ParseError`).
  4. In `core/CMakeLists.txt` add a custom command reading `data/clarinet_resonators.json` and writing `${CMAKE_CURRENT_BINARY_DIR}/embedded_resonators.cpp` with `embeddedResonatorJson()` (raw string literal chunks ≤ 16 kB); remove the stub definition.
  5. Run `pytest tests/python/test_resonator_tmm.py -k t031`, `clar_unit_tests "[T-006]"`, `clar_unit_tests "[T-026]"`.
- Outputs: tools/resonator/generate_table.py, data/clarinet_resonators.json, core/src/ResonatorTable.cpp, core/CMakeLists.txt
- Evidence produced: T-006, T-031, T-026 (structure; cents windows may need S-009)
- Done when: T-006 and T-031 pass; T-026 failures, if any, are only tuning windows.
- Checkpoint: step, commit, table SHA-256.
- On failure: decision rule "T-026".
- Gate: none
- Relevant decisions/claims: D-009, C-013, C-025, C-027, C-032

### S-008 Real-time voice core
- Tier: Opus
- Profile: software
- Depends on: S-005, S-007
- Inputs: ClarinetVoice.h; D-005, D-010, D-011, D-014
- Actions:
  1. Create `core/src/VoiceTuning.h` with all non-frozen constants of D-006/D-011/D-014.
  2. Implement `ClarinetVoice::Impl`: resolveNote → table entry; oversampled exciter + modal loop (D-005, f_r rule), half-band decimators, parameter smoothing, note state machine (D-011), output stage (D-014), UI atomics (D-010); implement `clamped()`.
  3. Run `clar_operational_tests`, `clar_alloc_tests`, `clar_integration_tests "[T-020]"`.
- Outputs: core/src/ClarinetVoice.cpp, core/src/VoiceTuning.h (+ private helpers)
- Evidence produced: T-014, T-015, T-020, T-021, T-025
- Done when: those pass.
- Checkpoint: step, commit.
- On failure: NaN in T-014 → reset state when |p| > 50 or non-finite (log); allocation → decision rule; T-020 → decision rule.
- Gate: none
- Relevant decisions/claims: D-005, D-010, D-011, D-014, C-008, C-032

### S-009 Tuning calibration
- Tier: Sonnet
- Profile: software
- Depends on: S-008
- Inputs: D-008
- Actions:
  1. Create `tools/render/CMakeLists.txt`; add `add_subdirectory(tools/render)` after `add_subdirectory(core)` in the root CMakeLists. Add `tools/render/calibrate_tuning.cpp` → target `clar_calibrate` (links clar_core) that loads `data/clarinet_resonators.json`, calibrates each entry per D-008 and rewrites `tuning_scale` (stable formatting as S-007).
  2. Run it; rebuild (re-embeds the table).
  3. Run `clar_integration_tests "[T-010]"` and `clar_unit_tests "[T-026]"`.
- Outputs: tools/render/CMakeLists.txt, tools/render/calibrate_tuning.cpp, CMakeLists.txt, data/clarinet_resonators.json
- Evidence produced: T-010, T-026
- Done when: both pass.
- Checkpoint: step, commit, max |cents| per entry.
- On failure: decision rule "T-010".
- Gate: none
- Relevant decisions/claims: D-008, C-013, C-029

### S-010 Expression and MIDI behaviour
- Tier: Sonnet
- Profile: software
- Depends on: S-009
- Inputs: D-011
- Actions: implement breath/velocity, legato stack (fixed array of 16 held notes), portamento interpolation, release/idle detection, vibrato, pitch bend (incl. f_r tracking); run `clar_integration_tests "[T-019]"` and all previously green core tests.
- Outputs: core/src/ClarinetVoice.cpp
- Evidence produced: T-019
- Done when: T-019 passes, no regression.
- Checkpoint: step, commit.
- On failure: default rule.
- Gate: none
- Relevant decisions/claims: D-011

### S-011 Registers, Overblow and Overblown fingering
- Tier: Opus
- Profile: software
- Depends on: S-010
- Inputs: D-006, D-007, D-009, C-004, C-022, C-027, C-033, C-035
- Actions: implement the D-006 mapping (κ₁ on closed/altissimo entries), D-007 forced κ₁ = 10, and verify the
  register entries play the twelfth; run `clar_integration_tests "[T-011],[T-012],[T-013]"` and all core tests.
- Outputs: core/src/ClarinetVoice.cpp, core/src/VoiceTuning.h
- Evidence produced: T-011, T-012, T-013
- Done when: those pass with no regression.
- Checkpoint: step, commit, final constants.
- On failure: decision rules for T-011, T-012 (clarion, legato, overblown), T-013.
- Gate: none
- Relevant decisions/claims: D-006, D-007, D-009, D-011

### S-012 State persistence
- Tier: Sonnet
- Profile: software
- Depends on: S-003
- Inputs: State.h, D-013
- Actions: implement with nlohmann/json (`parse(text, nullptr, false)`; size check first; floats via `is_number()`, bool via `is_boolean()`; then `clamped()`); run `clar_unit_tests "[T-017]"`.
- Outputs: core/src/State.cpp
- Evidence produced: T-017
- Done when: passes.
- Checkpoint: step, commit.
- On failure: default rule.
- Gate: none
- Relevant decisions/claims: D-013

### S-013 Plugin processor
- Tier: Sonnet
- Profile: software
- Depends on: S-011, S-012
- Inputs: plugin/src/PluginProcessor.*, D-010, D-011, D-013
- Actions: own a `clar::ClarinetVoice` (table from `embeddedResonatorJson()` loaded once in the constructor); `prepareToPlay` → `voice.prepare`; `processBlock`: `juce::ScopedNoDenormals`, read APVTS atomics into `VoiceParameters`, split the block at MIDI events and dispatch per D-011, render mono, copy to all channels; `setLatencySamples` with the decimator latency; state per D-013; `currentKeys()` from the voice. Run `xvfb-run -a build/plugin/clar_plugin_tests "[T-029]"`.
- Outputs: plugin/src/PluginProcessor.cpp
- Evidence produced: T-029 (parameter, sound and state cases)
- Done when: those cases pass; the editor case passes its processor assertions.
- Checkpoint: step, commit.
- On failure: default rule.
- Gate: none
- Relevant decisions/claims: D-010, D-011, D-013

### S-014 Editor and clarinet drawing
- Tier: Sonnet
- Profile: software
- Depends on: S-013
- Inputs: D-016, KeyLayout
- Actions:
  1. Implement `ClarinetView` (D-016), `setHighlightedKeys` (store + repaint on change), editor layout/controls/attachments, 60 Hz timer, `refreshFromProcessor`.
  2. Add `tools/render/snapshot_ui.cpp` → target `clar_ui_snapshots` in `plugin/CMakeLists.txt` (links ClarinetVST) writing the G-004 screenshots into `gates/G-004/`; on Linux run `xvfb-run -a build/plugin/clar_ui_snapshots`.
  3. Run all T-029 cases; build the G-004 bundle per plan/GATES.md.
- Outputs: plugin/src/{ClarinetView,PluginEditor}.cpp, plugin/CMakeLists.txt, tools/render/snapshot_ui.cpp, gates/G-004/*
- Evidence produced: T-029 (editor case)
- Done when: all T-029 cases pass; G-004 bundle complete.
- Checkpoint: step, commit.
- On failure: default rule.
- Gate: **G-004**
- Relevant decisions/claims: D-004, D-016, C-010, C-011

### S-015 Offline renderer and TinySOL tooling
- Tier: Sonnet
- Profile: software
- Depends on: S-011
- Inputs: tools/render/main.cpp usage line; D-015, D-021; C-005
- Actions:
  1. Implement `clar_render` (argument parsing, mono float32 WAV writer, exit codes as documented); add target `clar_render` (core only) to `tools/render/CMakeLists.txt`.
  2. Implement `tinysol.download` (Zenodo REST `https://zenodo.org/api/records/3685367` → file links + md5; streamed download; verify; extract; idempotent), `clarinet_notes`, and `compare_tinysol.main` per D-015/D-021.
  3. Download to `reference-data/tinysol` (git-ignored); record md5s, clarinet row count, pitch-offset distribution and the CC BY 4.0 attribution in `data/REFERENCE_DATA.md`.
- Outputs: tools/render/main.cpp, tools/render/CMakeLists.txt, tools/realism/{tinysol,compare_tinysol}.py, data/REFERENCE_DATA.md
- Evidence produced: none directly (enables T-022b)
- Done when: `CLAR_RENDER=build/tools/render/clar_render TINYSOL_DIR=reference-data/tinysol python -m pytest tests/python/test_realism_vs_tinysol.py` reaches the threshold assertions.
- Checkpoint: step, commit, number of clarinet notes.
- On failure: decision rules "TinySOL metadata", "Zenodo unreachable".
- Gate: none
- Relevant decisions/claims: D-015, D-021, C-005

### S-016 Realism calibration
- Tier: Opus
- Profile: software
- Depends on: S-014, S-015
- Inputs: results.json (T-022b); T-032; VoiceTuning.h
- Actions:
  1. Run T-022b and T-032; write `logs/S-016-round-<k>.md` with per-dynamic means and the worst 10 notes (split by register: chalumeau, throat, clarion, altissimo).
  2. Adjust only non-frozen constants (D-006, D-011, D-014, ζ map); after each round re-run all frozen tests (no regression). Max 5 rounds.
  3. Build the G-003 bundle per plan/GATES.md (12 blind pairs chosen with `random.Random(20261002).sample` over notes present in TinySOL at mf).
- Outputs: core/src/VoiceTuning.h, logs/S-016-*.md, gates/G-003/*
- Evidence produced: T-022b, T-032
- Done when: T-022b and T-032 pass or 5 rounds exhausted; G-003 bundle complete.
- Checkpoint: step, round, metrics.
- On failure: decision rules "T-022b", "T-032".
- Gate: **G-003**
- Relevant decisions/claims: D-014, D-015, A-012, C-015

### S-017 Performance and plugin validation on all platforms
- Tier: Sonnet
- Profile: software
- Depends on: S-016
- Inputs: CI jobs `core`, `plugin`
- Actions: push; get CI `core` (incl. `ctest -L perf`) and `plugin` (incl. pluginval strictness 10, auval) green on all three OSes; fix issues.
- Outputs: fixes as needed
- Evidence produced: T-016, T-023, T-029 on 3 OSes
- Done when: CI jobs `freeze`, `core`, `python`, `plugin`, `realism` green on the same commit.
- Checkpoint: step, commit, CI run URL.
- On failure: decision rules "T-016", "pluginval".
- Gate: none
- Relevant decisions/claims: D-020, C-019

### S-018 Final verification and report
- Tier: Sonnet
- Profile: software
- Depends on: S-017
- Inputs: all
- Actions: `bash tests/scripts/verify_freeze.sh`; full local run of all suites; confirm warning-free core in CI logs; update README "Status" and "Building" (no claims beyond evidence); write `REPORT.md` (tests by ID with status, deviations, gate outcomes, open risks). Push.
- Outputs: README.md, REPORT.md
- Evidence produced: all T-IDs green on one commit
- Done when: everything green and REPORT.md pushed.
- Checkpoint: final.
- On failure: default rule.
- Gate: none (merging to `main` is the human's decision, D-019)
- Relevant decisions/claims: all
