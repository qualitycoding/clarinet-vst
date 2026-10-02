# Human gates

Rule-10 required gates:
- **G-001** — N/A (`math`/`computational` inactive).
- **G-002** — N/A: no external or irreversible action (no release, package publication, archive deposit or
  submission; CI artifacts expire after 14 days). A public release later requires re-running the protocol
  with `software.deploys = true`.

Risk-mitigation gates (Phase 4.4):

## G-004 Fingering chart and UI check
- **Trigger:** end of S-014.
- **Evidence bundle** (`gates/G-004/`): 45 PNG screenshots `written-<NN>-<name>.png` of the editor
  highlighting each standard fingering (headless `juce::Component::createComponentSnapshot`), 14 screenshots of
  overblown-fingering mode (concert 69–82), the fixture file, and `GATE-G-004.md` listing the D-004 choices
  (LH E for E3/B4, RH F for F3/C5, LH F♯ for F♯3/C♯5, side key 4 for E♭4/B♭5, alternate-chart fingerings for
  G♯6–C7, throat B♭4 = register + A key).
- **Questions:** (1) Are all highlighted fingerings correct for your clarinet practice? (2) Any lever drawn in
  the wrong place or missing? (3) Do you want different choices for the notes with several standard fingerings?
- **Allowed responses:** `proceed` | `fix-layout: <levers>` (non-frozen `KeyLayout.cpp` geometry only) |
  `correct-chart: <notes>` (frozen fixture → `TEST_CHALLENGE.md`, protocol re-run) | `stop`.
- **Branches:** proceed → S-015; fix-layout → amend, re-run T-018/T-029, re-issue G-004; correct-chart → halt
  per Test Challenge Rule; stop → write final report, halt.

## G-003 Realism sign-off
- **Trigger:** end of S-016.
- **Evidence bundle** (`gates/G-003/`): `results.json` from T-022b and its pass/fail summary; T-032 output;
  `blind/` with 12 pairs `pair-NN-A.wav`/`pair-NN-B.wav` (one TinySOL clarinet note, one render of the same
  pitch/dynamic; A/B order from `random.Random(20261002)`), answer key `blind/key.json.b64`; `demos/` (44.1 kHz
  WAV): chromatic scale concert D3–B♭6 at mf, a legato phrase crossing the break (G4–A4–B♭4–B4–C5) with
  vibrato, an overblow sweep 0→1 on concert F3, overblown-fingering run concert 69–82, an altissimo scale
  concert B5–B♭6, pp/mf/ff of concert G4; `GATE-G-003.md` summarising metrics and deviations.
- **Questions:** (1) Without the key, which file in each pair is the real clarinet? (2) Is the sound
  acceptable as "realistic" for your use? (3) Is the Overblow / Overblown-fingering behaviour what you wanted?
  (4) Is the altissimo acceptable (it uses the D-009 approximation)?
- **Allowed responses:** `proceed` | `iterate: <notes>` (max 3 iterations of S-016) |
  `proceed-with-rescope: <text>` | `stop`.
- **Branches:** proceed → S-017; iterate → S-016 with notes, then G-003 again; rescope → record A-0xx,
  adjust only non-frozen constants/scope, continue S-017; stop → final report, halt.
