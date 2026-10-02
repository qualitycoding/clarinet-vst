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

Planning. The execution plan lives on the `gen-*-clarinet-vst-plan` branch (start with `HANDOFF.md`).
Nothing is implemented on `main` yet.

## Building

Not yet buildable from `main`. Once implemented: CMake ≥ 3.22, a C++20 compiler, and an internet
connection for the pinned JUCE / Catch2 / nlohmann-json downloads:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Licence

Source code: [Apache License 2.0](LICENSE). See [NOTICE](NOTICE).
Third-party components (JUCE, VST 3 SDK, etc.) keep their own licences; binaries built with JUCE are
subject to the JUCE licence you build under (AGPLv3 or a commercial JUCE licence).
