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

Implemented on the `impl/clarinet-v1` branch (not merged to `main` yet). See [REPORT.md](REPORT.md) for what works, the test
results and the open items: the realism of the sound against real clarinet recordings is **not yet signed off**
([GATE-G-003.md](GATE-G-003.md)), and two frozen tests await corrections ([TEST_CHALLENGE.md](TEST_CHALLENGE.md)).
The plan is in `plan/` (start with `HANDOFF.md`).

## Building

CMake ≥ 3.22, a C++20 compiler and an internet connection for the pinned JUCE / Catch2 / nlohmann-json downloads
(Linux also needs the JUCE system packages, see `plan/ENVIRONMENT.md`):

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure        # C++ tests
python -m pytest tests/python                      # Python tests (the TinySOL test needs a download)
```

Plugin bundles end up in `build/plugin/ClarinetVST_artefacts/Release/`. Use `-DCLAR_BUILD_PLUGIN=OFF` for the core library
and tools only.

## Licence

Source code: [Apache License 2.0](LICENSE). See [NOTICE](NOTICE).
Third-party components (JUCE, VST 3 SDK, etc.) keep their own licences; binaries built with JUCE are
subject to the JUCE licence you build under (AGPLv3 or a commercial JUCE licence).
