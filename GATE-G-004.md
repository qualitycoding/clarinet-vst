# GATE G-004 — Fingering chart and UI check

Triggered after S-014. Implementation is paused here until you answer (plan/GATES.md).

## Evidence (`gates/G-004/`)
- `written-NN-<note>.png` — 45 images, one per standard fingering, written E3 (52) … C7 (96). The caption under each
  picture shows the written note being fingered and the concert note that sounds.
- `overblown-concert-NN-<note>.png` — 14 images (concert 69–82): Overblown-fingering mode, where the picture shows
  the chalumeau fingering a twelfth below *without* the register key, and the plugin overblows it.
- `editor.png` — the full editor (picture, nine knobs, Overblown fingering toggle).
- `tests/fixtures/clarinet_standard_fingerings.txt` — the frozen fingering table behind the pictures.

Amber = lever pressed / hole covered. The thumb hole and the register key are on the back of the clarinet, so they
are drawn in the "rear view" box on the left.

## Choices I made where the chart offered several fingerings (D-004)
| Notes | Choice |
|---|---|
| E3 / B4 | left-hand E key (LH pinky) |
| F3 / C5 | right-hand F key |
| F#3 / C#5 | left-hand F#/C# key |
| E♭4 / B♭5 | side key 4 |
| G♯6 – C7 | first fingering listed in the Woodwind Fingering Guide's *alternate* chart (its basic chart stops at G6) |
| Throat B♭4 | register key + A key |
All fingerings come from one source (the Woodwind Fingering Guide), so a check by someone who plays is the safeguard.

## Questions
1. Are all highlighted fingerings correct for your clarinet practice?
2. Is any lever drawn in the wrong place or missing?
3. Do you want different choices for the notes that have several standard fingerings?

## Allowed responses
- `proceed` — continue with S-016 (realism calibration).
- `fix-layout: <levers>` — I move/redraw those levers (drawing code only); the gate is re-issued.
- `correct-chart: <notes>` — the frozen fingering table is wrong for those notes. This halts implementation under the
  Test Challenge rule: you approve the corrected table, I update the frozen fixture and manifest.
- `stop`.

## Other open items (not part of this gate)
- T-001 correction awaiting approval — see `TEST_CHALLENGE.md`.
- Real-recording realism checks (T-022b) need zenodo.org reachable (CI, or allow the host in the sandbox network).
