# clarinet-vst

A physically modelled B♭ clarinet instrument plugin (VST3 / Standalone, plus AU on macOS).

- **Sound:** a single-reed exciter coupled to a modal model of a clarinet-like cylindrical bore with a
  register hole and a tone-hole lattice — no samples.
- **UI:** draws a Boehm-system clarinet and highlights the keys, rings and tone holes of the fingering
  for the note being played (written and concert note names shown).
- **Overblowing:** an *Overblow* control (harder, brighter, until the note breaks to the twelfth or
  squeaks) and an *Overblown fingering* switch that plays clarion notes as overblown chalumeau
  fingerings without the register key.

## Status

`main` contains only the licence and this file. The plugin is implemented on the **`impl/clarinet-v1`**
branch, which has not been merged. Its `REPORT.md` is the detailed status. In short:

- Built: a physical model (single-reed exciter on a modal bore generated offline, 45 resonators covering
  written E3 to C7), a drawn Boehm clarinet that highlights the fingering, the Overblow control and
  overblown-fingering switch, monophonic legato, breath/velocity, pitch bend and aftertouch vibrato.
  Formats are VST3 and Standalone on Windows, macOS and Linux, plus AU on macOS.
- Tuning, real-time safety, state save/restore and the supply-chain checks are met. CI builds the plugin
  on all three systems.
- **Not signed off:** realism. Against 126 real clarinet notes (TinySOL), the model's harmonic error
  is about 9 dB on average, against a 6 dB target, so the realism test fails and the sound has not been
  accepted by ear (gate G-003).
- Two frozen tests need corrections: one expected pitch constant is wrong, and the realism attack
  criterion is infeasible (`TEST_CHALLENGE.md` on that branch).

The planning documents are on `gen-20261002T075147Z-clarinet-vst-plan` (start with `HANDOFF.md`).

## Building

Build from `impl/clarinet-v1`, not `main`. You need CMake 3.22 or later, a C++20 compiler and an
internet connection for the pinned JUCE, Catch2 and nlohmann-json downloads (Linux also needs the JUCE
system packages; see `plan/ENVIRONMENT.md` on that branch):

```bash
git checkout impl/clarinet-v1
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Plugin bundles end up in `build/plugin/ClarinetVST_artefacts/Release/`.

## Licence

Source code: [Apache License 2.0](LICENSE). See [NOTICE](NOTICE).
Third-party components (JUCE, VST 3 SDK, etc.) keep their own licences; binaries built with JUCE are
subject to the JUCE licence you build under (AGPLv3 or a commercial JUCE licence).
