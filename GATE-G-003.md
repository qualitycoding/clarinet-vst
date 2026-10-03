# GATE G-003 — Realism sign-off

Triggered after S-016. Implementation of S-017/S-018 continues in parallel only for items that don't depend on your
answer; the final report waits for it.

## Evidence
Everything is on the branch **`ci-g003`** (built by the CI `realism` job from the TinySOL clarinet recordings):
<https://github.com/qualitycoding/clarinet-vst/tree/ci-g003> — download it all with
<https://github.com/qualitycoding/clarinet-vst/archive/refs/heads/ci-g003.zip>

- `blind/pair-01-A.wav` … `pair-12-B.wav` — 12 pairs. In each pair one clip is a real clarinet note (TinySOL, mf,
  steady part, level-matched) and the other is the plugin playing the same pitch. Which is which is random.
  The answer key is `blind/key.json.b64` (base64 — **do not decode it until you've answered**).
- `demos/` — 01 chromatic scale D3–B♭6 · 02 legato phrase across the break with vibrato · 03 Overblow sweep on F3 ·
  04 overblown-fingering run (no register key) · 05 altissimo scale B5–B♭6 · 06 pp / mf / ff on concert G4.
- `SUMMARY.md` and `results.json` — the objective comparison (below).

## Objective results (126 real notes; frozen thresholds from the plan)
| Dynamic | harmonic error, mean (target ≤ 6 dB) | worst note (target ≤ 10) | brightness within ±25 % of real (target ≥ 90 %) |
|---|---|---|---|
| pp | 9.7 dB | 26.6 dB | 95 % |
| mf | 8.9 dB | 25.4 dB | 50 % |
| ff | 9.3 dB | 16.9 dB | 55 % |

Louder playing is brighter than softer playing for 93 % of pitches (target ≥ 90 % — met). The clarinet's odd-harmonic
fingerprint test passes. **The harmonic and brightness targets are not met**, and the attack criterion cannot be met by
any plausible sound (`TEST_CHALLENGE.md`: the recorded attack times vary 8–34× between notes). Details of the five
calibration rounds: `logs/S-016-fit.md`. Closest to real: the low (chalumeau) and throat registers; weakest: clarion
and especially altissimo, where the plugin uses a simplified resonator.

## Questions
1. Without the key, which file in each pair is the real clarinet? (If you can't tell, that is the best result.)
2. Is the sound realistic enough for your use?
3. Is the Overblow / Overblown-fingering behaviour what you wanted (demos 03 and 04)?
4. Is the altissimo acceptable (demo 05)?

## Allowed responses
- `proceed` — accept the sound; continue to S-017 / S-018.
- `iterate: <notes>` — I do up to three more calibration rounds following your notes (e.g. "pp too bright", "clarion
  too thin"), then re-issue this gate.
- `proceed-with-rescope: <text>` — accept with a changed scope (recorded as a new assumption).
- `stop`.

## Also needing your decision
- **T-001** (wrong frozen constant, 1872.4721 → 1873.1307508272341) and **T-022b attack criterion** — see
  `TEST_CHALLENGE.md`. Approving them lets CI go green.
