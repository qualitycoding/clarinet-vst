# HANDOFF — clarinet-vst v1 (implementing agent: start here)

**Purpose.** Build a B♭ clarinet physical-model instrument plugin (VST3/AU/Standalone) whose UI draws a
Boehm clarinet and highlights the levers (keys, rings, holes) of the note being played, with an **Overblow**
knob and an **Overblown fingering** switch, and prove realism objectively and by human sign-off. You execute
`plan/PLAN.md`; you do not re-plan.

**Active profile:** `software` (deploys = false). See `plan/PROFILE.md` for N/A sections.

## Reading order
1. This file → 2. `plan/PROFILE.md` → 3. `plan/ASSUMPTIONS.md` → 4. `plan/DECISIONS.md` (design, interfaces,
decision rules) → 5. `plan/PLAN.md` (S-001…S-018) → 6. `plan/GATES.md` → 7. `plan/ENVIRONMENT.md` →
8. `plan/TRACEABILITY.md` → 9. `premortem/RISK_REGISTER.md`. Background only: `research/`
(claims, sources, rounds, spikes C1–C11 — the spikes are reference implementations for S-005/S-006).

## The three things that differ from a naive design (read D-009, D-006, D-005)
1. A linear modal model does **not** make clarion fingerings speak the twelfth (C-027). The table generator
   multiplies the first-mode damping of register-hole entries by 10 (nonlinear-loss emulation) and the voice
   uses reed damping q_r = 0.7. Verified by spikes C3/C4/C8/C9 and frozen T-009/T-012.
2. Closed fingerings never overblow by blowing harder alone (C-033); the Overblow knob suppresses mode 1
   above o = 0.6 (κ₁), and the Overblown-fingering switch forces it.
3. The altissimo (above the 1.5 kHz lattice cutoff) uses a plain short cylinder and a reed resonance
   f_r = max(1500 Hz, 2·f_note) (C-032).

## Environment
Follow `plan/ENVIRONMENT.md` exactly (system packages, Python venv from `tools/requirements.txt`, CMake ≥ 3.22).

## Running the frozen suite
```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build -j1
ctest --test-dir build -LE perf --output-on-failure     # unit, integration, operational, alloc (+ plugin under xvfb)
ctest --test-dir build -L perf --output-on-failure      # T-016 (Release only)
xvfb-run -a build/plugin/clar_plugin_tests              # T-029 on headless Linux
python -m pytest tests/python -k "not tinysol"          # T-022a, T-024, T-030, T-031
CLAR_RENDER=build/tools/render/clar_render TINYSOL_DIR=reference-data/tinysol \
  python -m pytest tests/python/test_realism_vs_tinysol.py   # T-022b (network once)
xvfb-run -a bash tests/scripts/run_pluginval.sh build/plugin/ClarinetVST_artefacts/Release/VST3/Clarinet.vst3  # T-023
```

## Verifying the freeze
`bash tests/scripts/verify_freeze.sh` (= `sha256sum --check --strict tests/FROZEN_MANIFEST.sha256`) must print
`freeze OK` before and after every step. Frozen files carry a `FROZEN — DO NOT MODIFY` header (the JSON fixture
`tests/fixtures/resonators_valid.json` cannot carry a comment; it is frozen through the manifest).

## Steps at a glance
S-001 env/baseline → S-002 CI + notices → S-003 pitch/levers/fingering/layout → S-004 analysis →
S-005 reed + reference integrator → S-006 TMM (Python) → S-007 resonator table → S-008 real-time voice →
S-009 tuning calibration → S-010 expression/MIDI → S-011 registers/Overblow/Overblown fingering →
S-012 state → S-013 plugin processor → S-014 editor/UI → **G-004** → S-015 renderer + TinySOL tooling →
S-016 realism calibration → **G-003** → S-017 CI on 3 OSes (perf, pluginval) → S-018 final report.
Independent after S-001: S-003 ∥ S-004 ∥ S-006 ∥ S-012.

## Human gates
- **G-004** after S-014: fingering chart and UI screenshots.
- **G-003** after S-016: blind listening + realism metrics + overblow and altissimo questions.
At a gate: stop, write `GATE-<id>.md` (evidence summary, questions, allowed responses from `plan/GATES.md`), push,
wait. G-001 and G-002 are N/A.

## Halt / deviation protocol
- Anything not covered by a decision rule: apply the **default rule** in `plan/DECISIONS.md`, log it in
  `DEVIATIONS.md` (step, situation, choice, rationale).
- Halt and write `BLOCKED.md` if a change would touch frozen tests/fixtures, security, data integrity, a public
  interface (headers/stub signatures/module constants), or research integrity.
- If a frozen test seems invalid: halt, write `TEST_CHALLENGE.md` (test ID, evidence, proposed fix). Never modify,
  skip, `[!mayfail]`, `xfail` or weaken a frozen test.
- Never create a release, tag, package, or publish anything. Never commit `reference-data/`, build trees, or any
  credential. Use your own GitHub credential (environment), never one copied from planning logs.

## Integrity rule (Rule 9, verbatim)
**Integrity** `[All]`: No step may fabricate, cherry-pick without disclosure, or manually alter data, test results, benchmarks, or figures. In addition:
* `[computational, publication]` Every reported number is generated from committed results, not transcribed by hand.
* `[publication]` Generative-AI images are never used as data figures. AI assistance is disclosed according to the venue's policy, as recorded in `plan/ASSUMPTIONS.md`.

(The bracketed sub-rules are inactive for this software-only plan, but realism metrics and gate bundles must
still be produced only by the committed tools.)
