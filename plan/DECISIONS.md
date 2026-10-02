# Decisions, interfaces and decision rules

## Design decisions

**D-001 Framework.** JUCE 9.0.3 (commit `be29c814…`), CMake ≥ 3.22, C++20 (C-002). JUCE is consumed by
FetchContent, never vendored.

**D-002 Pinning.** Every FetchContent dependency is pinned to a full 40-hex commit SHA (JUCE, Catch2 v3.16.0,
nlohmann/json v3.12.0; C-018). Python tools: exact `==` pins in `tools/requirements.txt` + full lock
`tools/requirements.lock`. GitHub Actions pinned to SHAs (D-020). Enforced by T-024.

**D-003 Architecture.**
- `core/` → static library `clar_core`: pure C++20, no JUCE: pitch, levers, fingering, key-layout geometry,
  reed model, reference integrator, resonator table, real-time voice, analysis, state. Testable headless.
- `plugin/` → `juce_add_plugin(ClarinetVST)`: parameters, MIDI → `ClarinetVoice`, editor drawing.
- `tools/render/` → `clar_render` CLI (core only); `tools/resonator/` (Python) generates
  `data/clarinet_resonators.json`; `tools/realism/` (Python) compares renders with TinySOL.
- The resonator table is embedded into `clar_core` at build time (D-009).

**D-004 Fingering chart.** Standard fingerings for written 52–96 frozen in
`tests/fixtures/clarinet_standard_fingerings.txt` (C-010): WFG basic chart, first-listed fingering, for
E3–G6; WFG alternate chart (upper altissimo), first-listed fingering, for G♯6–C7 (C-034, A-021). Lever set
of 24 (C-011, `Keys.h`). Choices where the basic chart lists several: E3/B4 use LH E; F3/C5 use RH F;
F♯3/C♯5 use LH F♯; E♭4/B♭5 use side key 4. `targetRegister`: 1 for 52–70, 2 for 71–84, 3 for 85–96.
`vent`: none for 52–70 (throat B♭4 uses register key + A key but is modelled as a short closed resonator),
register for 71–84, altissimo for 85–96. Invariants: T-004.

**D-005 Synthesis model and scheme.** Exciter = C-006 equations (contact damping β of Petersen omitted, as in
Colinot 2021). Resonator = modal sum of the table entry (complex residues). Integration (identical to the
sibling plan; verified by T-008):
- modal states: `p_n ← e^{s_n Δt} p_n + C_n (e^{s_n Δt} − 1)/s_n · u` (flow held over the step);
- reed: central differences `x_{k+1} = (Δt²ω_r²(p − γ + F_c − x_k) + 2x_k − x_{k−1} + ½q_rω_rΔt x_{k−1}) / (1 + ½q_rω_rΔt)`;
- `p = 2 Σ Re(p_n)` evaluated before the reed update.
Internal rate `F_int = K·F_host`, `K = clamp(ceil(176400 / F_host), 1, 8)` rounded up to a power of two (C-008);
decimation by cascaded half-band FIR (≥ 90 dB stop-band, linear phase, latency ≤ 64 host samples, reported).
Reed: `clarinetReed()` (C-026) with `q_r` from D-011 and **`f_r = max(1500 Hz, 2·f_target)`** where f_target is
the sounding note's ET frequency (spike C7: needed for the altissimo; ≤ 750 Hz notes keep 1500 Hz).
`simulateColinot()` (offline) uses the same equations at a fixed rate (default 176 400 Hz).

**D-006 Overblow knob (o ∈ [0,1]).** Physically motivated (C-033, C-022, C-009); constants live in
`core/src/VoiceTuning.h`, are **not frozen** and may be retuned to satisfy T-011/T-013/G-003:
- `γ_target = min(0.95, γ_base·(1 + 0.35·o))`; `q_r = max(0.25, q_r,base − 0.4·o)` (tighter embouchure → squeakier);
- attack `τ_g = τ_base·(1 − 0.8·o)`;
- mode-1 suppression on `vent ∈ {none, altissimo}` resonators: `Re(s_1) ← κ₁·Re(s_1)`,
  `κ₁ = 1 + 9·smoothstep(0.6, 1.0, o)` → 1 up to o = 0.6, 10 at o = 1 (break to the twelfth, C-033);
- brightness shelf `+3 dB·o`; breath-noise gain `×(1 + o)`.
For `vent = register` entries (already emulated, D-009) only γ, q_r, τ, shelf and noise change.

**D-007 Overblown-fingering switch.** `resolveNote(n, true)` for written 71–84 shows the chalumeau fingering of
written−19 **without** the register key and uses resonator `(written−19, none)`. The voice then forces
`κ₁ = 10` on that resonator (closed-hole twelfth, C-033), keeps all other D-006 terms at the knob value.
Expected pitch error ±50 cents (not calibrated; T-012). Fallback: decision rule "T-012 overblown".

**D-008 Tuning.** Each table entry has `tuning_scale` (default 1.0) multiplying every Im(s_n). S-009
calibrates it per entry by rendering the entry's standard note at velocity 0.6, overblow 0, 48 kHz and
iterating `scale ← scale · 2^(−cents/1200)` until |cents| ≤ 2 (max 8 iterations; target = the note's ET
frequency; for register entries the measured f0 is the twelfth). Pitch bend and A4 multiply all Im(s_n)
by `2^(bend·2/12)·(A4/440)`; `f_r` follows the bent target.

**D-009 Resonator table and virtual clarinet.** Schema `clarinet-vst/resonators@1`:
```json
{"schema":"clarinet-vst/resonators@1","instrument":"Bb soprano clarinet",
 "units":{"pole":"rad/s","residue":"Z/Zc"},
 "provenance":{"generator":"tools/resonator/generate_table.py","git":"<sha>","geometry":"D-009",
               "nl_mode1_bw_factor":10.0},
 "entries":[{"written":52,"vent":"none","tuning_scale":1.0,
             "modes":[{"re_s":-18.2,"im_s":935.6,"re_c":714.0,"im_c":10.2}, ...]}]}
```
Validation (T-006): JSON object ≤ 4 MiB, schema tag, `vent ∈ {none, register, altissimo}`,
2 ≤ modes ≤ 24, `re_s < 0`, `im_s` strictly increasing, all numbers finite, unique (written, vent),
`tuning_scale` optional (default 1.0) in [0.8, 1.25]. Entries: (52–70, none), (71–84, register),
(85–96, altissimo) = 45.
Geometry (`tools/resonator/tmm.py` constants; C-028, C-030, C-016, C-031):
- bore radius 7.5 mm; lattice of 5 open holes with the Petersen fc = 1.5 kHz cell scaled by 7.5/6.5 in hole
  radius (b = 4.615 mm, chimney 9.8 mm, half-spacing ℓ = 16.3 mm; eq. (1) keeps fc), first lattice hole at
  L + ℓ, bore ends ℓ after the last hole, unflanged radiation;
- register hole at 140 mm, radius 1.55 mm, chimney 12.5 mm (closed for `none`, open for `register`);
- `none`: L solved by bisection (1e-6 m) so the first peak = `ET(written−2)·2^(25/1200)` (C-013);
- `register`: the `none` geometry of written−19 with the register hole open;
- `altissimo`: plain closed-open cylinder, radius 7.5 mm, no holes, length solved as for `none` (C-032).
Modal fit: all |Z| peaks in 20 Hz … `max(4000, 7.5·f1)` Hz, at most 24, at least 2; `modal_fit` refines poles
and complex residues. **Register-hole nonlinear-loss emulation** (C-027, C-035): for `register` entries
`Re(s_1) ← 10·Re(s_1)` after fitting; recorded as `provenance.nl_mode1_bw_factor = 10.0`.
Embedding: CMake reads `data/clarinet_resonators.json` and generates `embedded_resonators.cpp` exposing
`embeddedResonatorJson()` (raw string literal split into ≤ 16 kB chunks for MSVC).

**D-010 Real-time safety.** After `prepare()`: no allocation, no locks, no exceptions, no I/O in `process()`,
note/parameter setters, or `currentKeys()`. Parameters smoothed (one-pole, 10 ms). UI reads an
`std::atomic<uint32_t>` key bitmask (24 bits) and `std::atomic<int>` note, written once per block.
Denormals: `juce::ScopedNoDenormals` in the plugin; 1e-20 anti-denormal offset in modal states.

**D-011 Expression.** Breath absent until a breath value ≥ 0 arrives (`setBreath(-1)` resets).
Absent: `γ_base = 0.45 + 0.30·velocity` (pp 0.53, mf 0.63, ff 0.74; C-007); present:
`γ_base = breath < 0.02 ? 0 : 0.43 + 0.32·breath`. `ζ = 0.40 − 0.15·reedHardness` (C-007 range).
`q_r,base = 0.7` (C-022: avoids squeaks at normal playing). Attack `τ_base = 8 ms` (spikes C4/C9: twelfths
robust 3–30 ms). Velocity scales output by `0.5 + 0.5·velocity` only when breath is absent. Vibrato:
γ and pole scaling at `vibratoRateHz`, depth `vibratoDepth·(0.3 + 0.7·aftertouch)`, max ±15 cents.
Legato note change: resonator parameters (poles, residues, f_r) interpolated linearly over `portamentoMs`
(min 3 ms). Note off: γ ramps to 0 over 40 ms; when output RMS < 1e-5 for 50 ms the voice is idle
(note −1, keys empty). MIDI: omni; CC2/CC11 → breath; channel pressure → aftertouch; pitch bend ±2 st;
CC120/CC123 → allNotesOff; note-on velocity 0 = note-off.

**D-012 Stub conventions.** Non-noexcept stubs throw `clar::NotImplemented` (C++) / `NotImplementedError`
(Python). Noexcept stubs return the sentinel documented in the header. Tests fail by assertion or exception.

**D-013 Parameters and state.** Permanent parameter IDs: `overblow, overblown_fingering, reed_hardness,
brightness, breath_noise, vibrato_rate, vibrato_depth, portamento, tuning_a4, output_gain` (ranges as in
`VoiceParameters`). State blob = UTF-8 JSON `{"schema":"clarinet-vst/state@1","<VoiceParameters field>": value, …}`
from `serializeState()`; `setStateInformation` parses with `deserializeState()`, keeps current values on `nullopt`.

**D-014 Output stage.** Mouthpiece pressure → radiation filter: first-order high-pass 100 Hz × peaking EQ at
1.5 kHz (Q 0.7, +3 dB, lattice "reinforced band", C-015) × peaking EQ at 2.1 kHz (Q 2, +2 dB, bell) ×
high-shelf at 3 kHz with gain `−6 dB + 9 dB·brightness` → + breath noise (xorshift32, seed 0x5A5A5A5A at
`prepare()`, band-pass 1–6 kHz, gain `breathNoise·0.05·|u|`) → DC blocker (5 Hz) → `outputGainDb` → `tanh`
→ |y| ≤ 1. Mono copied to all output channels. All constants are tuning constants (not frozen; S-016).

**D-015 Realism procedure (frozen procedure, A-012 thresholds).** For every row of
`tinysol.clarinet_notes()`: determine the reference concert MIDI per D-021; render
`clar_render --note <concert> --velocity {pp:.25, mf:.6, ff:.95} --seconds <ref duration capped at 4 s>
--fs 44100 --release-at <ref duration − 0.3 s>`; resample the reference to 44.1 kHz if needed; compute
`metrics.compare(ref, syn, fs)` on steady segments using the measured reference f0 for harmonic picking;
write one JSON row per note `{midi, dynamics, harm_mad_db, centroid_ratio, attack_ratio, centroid_syn_hz,
f0_ref, f0_syn, pitch_offset}` sorted by (midi, dynamics).

**D-016 UI.** Editor 900×600 (resizable, fixed aspect, 0.75–2×). Left 60 %: `ClarinetView` — upright clarinet
(mouthpiece top, bell bottom) drawn with `juce::Path`: barrel, upper joint (LH holes, A and side-G♯ keys
near LH1, LH Eb sliver beside LH2, LH pinky cluster C♯/E/F/F♯ below LH3), lower joint (RH holes, B sliver,
side keys 1–4 to the right of RH1–RH2, RH pinky cluster E/F/F♯/G♯ below RH3), bell. A "rear" inset left of
the upper joint shows the thumb ring hole and the register key. Ring holes drawn as circles with a ring;
pressed levers filled `#F2B134`, others outlined `#8A8F98`. Below: written + concert note name and register
label (chalumeau/throat/clarion/altissimo/“overblown”). Right 40 %: 9 rotary knobs + "Overblown fingering"
toggle (APVTS attachments). Editor timer 60 Hz → `refreshFromProcessor()`.

**D-017 Third-party notices.** `THIRD_PARTY_NOTICES.md` lists: JUCE 9 (AGPLv3 / JUCE licence), VST 3 SDK
(MIT), nlohmann/json (MIT), Catch2 (BSL-1.0, tests only), pluginval (GPLv3, CI only), TinySOL (CC BY 4.0,
test data only, not distributed), Colinot et al. 2021 and Petersen et al. 2020 (CC BY 4.0; equations,
parameters, geometry), Debut et al. 2005 (geometry values, cited), Woodwind Fingering Guide (fingering
facts, cited), OpenWInD (GPLv3; optional cross-check only, never imported by committed code).

**D-018 Guard tests.** Pass at freeze time by design (documented exception to red verification): T-024a
(`test_cmake_dependencies_pinned_to_full_sha`), T-024b (`test_python_requirements_pinned`), T-028
(`verify_freeze.sh`), the T-029 case "all D-013 parameters exist with stable IDs", and T-023 (pluginval on
the stub plugin) if `research/spikes/plugin_build.log` records SUCCESS. Every other test fails against stubs.

**D-019 Branching.** Implementation branch `impl/clarinet-v1` from the generation-branch head; one commit
per step `S-0xx: <title>`. Merging to `main` is the human's choice after G-003.

**D-020 CI** (`.github/workflows/ci.yml`, S-002). Actions pinned (C-020):
`actions/checkout@3d3c42e5aac5ba805825da76410c181273ba90b1` (v7.0.1),
`actions/setup-python@5fda3b95a4ea91299a34e894583c3862153e4b97` (v7.0.0),
`actions/cache@55cc8345863c7cc4c66a329aec7e433d2d1c52a9` (v6.1.0),
`actions/upload-artifact@043fb46d1a93c77aae656e7c1c64a875d1fc6a0a` (v7.0.1). `permissions: contents: read`. Jobs:
1. `freeze` (ubuntu): `bash tests/scripts/verify_freeze.sh`.
2. `core` (matrix ubuntu-24.04, macos-15, windows-2025; Release, `-DCLAR_BUILD_PLUGIN=OFF`): build, `ctest -LE perf`, `ctest -L perf`.
3. `python` (ubuntu): install `tools/requirements.txt`, `pytest tests/python -k "not tinysol"`, `pip-audit -r tools/requirements.txt`.
4. `plugin` (matrix as 2; Release, plugin ON): build; Linux under `xvfb-run -a`: `ctest -R plugin` and
   `tests/scripts/run_pluginval.sh <VST3>` (plain call on macOS/Windows, C-019); macOS: copy the AU to
   `~/Library/Audio/Plug-Ins/Components`, `auval -v aumu Clra Qcod`; upload VST3/Standalone artifacts (14 days).
5. `realism` (ubuntu, needs core): cache `reference-data/tinysol` keyed on the hash of `tools/realism/tinysol.py`;
   `CLAR_RENDER=build/tools/render/clar_render TINYSOL_DIR=reference-data/tinysol pytest tests/python/test_realism_vs_tinysol.py`; upload `results.json`.

**D-021 TinySOL pitch convention.** Whether TinySOL's `Pitch ID` is concert or written for the clarinet is
not documented (C-005). `compare_tinysol` measures each reference's f0 (YIN, steady segment) and sets
`concert = round(69 + 12·log2(f0/440))`; rows are kept only if `concert − PitchID ∈ {0, −2}` and
`50 ≤ concert ≤ 94`; `pitch_offset` (0 or −2) is written per row. Rows with any other offset are excluded and
counted in `results.json` metadata (`excluded_pitch_rows`).

## Public interfaces
Authoritative signatures: `core/include/clar/*.h`, `plugin/src/{PluginProcessor,PluginEditor,ClarinetView,Parameters}.h`,
the `tools/**/*.py` stubs (function names, signatures, docstrings, module constants) and the `clar_render`
usage line in `tools/render/main.cpp`. Changing a public signature requires `BLOCKED.md`. Private helpers are free.

## Decision rules (if → then)
- **Dependency fetch/install fails** → retry 3× with 30 s back-off; pinned SHA gone upstream → `BLOCKED.md` (never float a version).
- **Compiler warning in core** → fix (S-018 requires warning-free core on all CI OSes).
- **T-008 f0 outside [180, 195] Hz** → compare the port sample-by-sample with `research/spikes/reed_spike.py::run()`
  driven by `colinotDSharpTable2()` and `ReedParams{}` for the first 1000 samples (max relative diff < 1e-6); fix the port; never change the window.
- **T-030 Petersen f1/cutoff fails** → compare `input_impedance` with `research/spikes/tmm_spike.py::zin` on the same
  segments; the frozen spike is the reference implementation of the TMM equations; fix the port.
- **T-009 fails** → check the emulation is applied to `modes[0]` only and `q_r = 0.7`; if the reference integrator
  passes T-008 but T-009 still fails → `TEST_CHALLENGE.md` (the spike replay passed 6/6 vs 3/6).
- **T-026 cents/ratio window fails** → re-solve lengths with 1e-7 m tolerance; if still failing, adjust the
  +25-cent offset globally within +10…+40 (logged in `DEVIATIONS.md`).
- **T-010 fails after D-008 calibration** → 16 iterations; if > 10 cents remain, the entry is in an unstable
  regime: raise its `γ_base` floor by ≤ 0.05 (logged); still failing → `BLOCKED.md`.
- **T-011 fails (chalumeau leaves R1 at o = 0)** → in order: (1) raise `q_r,base` toward 1.0, (2) lengthen
  `τ_base` toward 15 ms, (3) lower the γ slope 0.30 → 0.25. Log each.
- **T-012 clarion fails (no twelfth)** → apply in order to `register` entries only, stop at the first step that
  passes, log each: (1) the voice multiplies `Re(s_1)` by an extra factor ≤ 2 (the table keeps factor 10,
  which T-031 freezes); (2) `q_r,base` → 1.0; (3) residue `C_1 × 0.5` in the voice. Still failing → `BLOCKED.md`
  (a different clarion resonator would change the table schema/generator contract).
- **T-012 legato case fails** → "register-change articulation": on a legato change whose register differs,
  dip γ to 0.6·γ for 5 ms and re-run the attack ramp (C-004); log.
- **T-012 overblown-fingering case fails** → raise forced κ₁ 10 → 20; if > 50 cents error remains, apply a
  per-note pole scale for overblown mode computed at prepare() (logged).
- **T-013 brightness fails** → shelf +3 dB·o → +5 dB·o and γ gain 0.35 → 0.45 (keep κ₁ = 1 below o = 0.6); log.
- **T-013 break fails (< 80 % break at o = 1)** → smoothstep start 0.6 → 0.5 and κ₁(1) 10 → 20; log.
- **T-032 fails** → reduce the 1.5 kHz peaking gain and verify the high-pass corner ≤ 100 Hz (output stage
  must not create even harmonics); check `tanh` drive (lower gain into the clipper); log.
- **T-020 fails (≥ 3 cents between sample rates)** → raise the internal-rate target to 352 800 Hz (K power
  of two) and recalibrate at 48 kHz; if T-016 then fails, per-K tuning keys are a schema change → `BLOCKED.md`.
- **T-016 RTF > 0.05** → profile; allowed: float32 modal states, SIMD over modes, K = 2 for F_host ≥ 88.2 kHz.
  Never drop modes below the D-009 fit or K below ceil(176400/F_host)/2.
- **T-015 allocation detected** → move it into `prepare()`; never disable the test.
- **T-022b fails** → up to 5 calibration rounds on D-006/D-011/D-014 constants (not frozen); then proceed to
  G-003 with failing metrics in the bundle (human decides). Never edit thresholds.
- **TinySOL metadata lacks a REQUIRED_COLUMNS entry or has no clarinet rows** → `BLOCKED.md`.
- **Zenodo unreachable** → retry 3×; CI realism job fails (does not skip); continue other steps.
- **pluginval fails** → fix the plugin; known pluginval/JUCE-9 incompatibility → strictness 8 + `DEVIATIONS.md` + `BLOCKED.md`.
- **Headless editor test crashes on Linux** → run under `xvfb-run -a`; never skip.
- **pip-audit reports a vulnerability** → bump that package to the nearest fixed version, update the lock, re-run Python tests; log.
- **Frozen test or fixture appears wrong** (e.g. human corrects a fingering at G-004) → halt, `TEST_CHALLENGE.md` (Rule 2E.4).
- **Default rule:** choose the most reversible option that does not expand scope, log it in `DEVIATIONS.md`, continue —
  unless it touches frozen tests, security, data integrity, a public interface, or research integrity → `BLOCKED.md`.
