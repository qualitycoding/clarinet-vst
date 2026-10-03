# TEST_CHALLENGE — T-001 (tests/core/test_pitch.cpp:29), raised during S-003

**Frozen assertion**
```cpp
CHECK(equalTemperedHz(94, 442.0) == Catch::Approx(1872.4721).epsilon(1e-6));
```
**Why it appears wrong.** The documented contract (core/include/clar/Pitch.h, D-001 "12-TET frequency in Hz:
a4Hz * 2^((midi-69)/12)") gives, for midi = 94 and A4 = 442 Hz, 442 · 2^(25/12) = **1873.13075083 Hz**.
The frozen constant 1872.4721 differs by 0.66 Hz (0.035 %, ~0.6 cent) — a transcription error when the
test was written, not a modelling disagreement.

**Independent evidence** (run 2026-10-02, Python 3.12 double precision):
- `442 * 2**(25/12)` = 1873.1307508272341
- `442 * (2**(1/12))**25` = 1873.1307508272362
- `440 * 2**(25/12)` = 1864.6550460723597 (the other frozen checks in the same test pass with the same formula:
  `equalTemperedHz(69) == 440` and `equalTemperedHz(50) == 146.83238395870376`).
- The C++ implementation returns 1873.13075082723412 (matches).

**Proposed fix (one constant):** `Catch::Approx(1873.1307508272341).epsilon(1e-9)` in place of
`Catch::Approx(1872.4721).epsilon(1e-6)` (the comment on that test already states a 1e-9 relative tolerance).

**Impact.** Only this single assertion fails; every other T-001 assertion and all other tests are
unaffected. Nothing in the design depends on the value. The test file and `tests/FROZEN_MANIFEST.sha256`
were NOT modified. Implementation continues on steps that do not depend on this assertion; T-001 stays red
until the human approves the correction (which requires re-hashing the manifest).

---

# TEST_CHALLENGE — T-022b attack criterion (tests/python/test_realism_vs_tinysol.py), raised during S-016

**Frozen assertion**
```python
THRESH["attack_ratio"] = (0.5, 2.0), "fraction_required": 0.9   # >= 90 % of notes per dynamic
```
where `attack_ratio = synthetic attack time / TinySOL attack time`, attack = 10 % -> 90 % of the maximum of the
10 ms RMS envelope (`tools/realism/metrics.attack_time_s`).

**Why it cannot be satisfied by any single (or per-dynamic) synthetic attack.** The first real run (CI `realism`
job, 126 TinySOL clarinet notes, commit 0c9af03, all notes concert pitch) gave these *reference* attack times:

| dynamic | q10 | median | q90 | q90 / q10 |
|---|---|---|---|---|
| pp | 0.060 s | 0.147 s | 2.06 s | 34 |
| mf | 0.180 s | 0.618 s | 1.94 s | 11 |
| ff | 0.221 s | 0.564 s | 1.87 s | 8.5 |

The recorded sustained notes contain slow swells (the 10 ms envelope rises from -30 dB to 0 dB over 0.3 s or more
in the stored `env_ref_db` arrays), so "10 % -> 90 % of the global maximum" measures the swell, not the
note onset, and varies by 8-34x between notes of the same dynamic. A ratio band of [0.5, 2] is a factor-4 window.
The best any one synthetic attack time per dynamic can do (computed from the 126 stored reference values):

| dynamic | best coverage, band x2 (frozen) | x4 band | x8 band |
|---|---|---|---|
| pp | 52 % (T = 0.15 s) | 69 % | 90 % |
| mf | 60 % (T = 0.74 s) | 88 % | 98 % |
| ff | 69 % (T = 0.52 s) | 90 % | 100 % |

So the 90 % requirement is unreachable even with an unmusically slow synthetic attack (0.5-0.7 s). Our
attack is 18-35 ms (a realistic clarinet articulation); matching the swell would make the plugin unusable for
normal playing. The criterion fails on the metric's design, not on the model.

**Proposed fix (human decides).** Either (a) widen the band to [0.125, 8.0] for >= 90 % (reachable for all three
dynamics: 90 / 98 / 100 %), or (b) change the attack metric to an onset-based one that ignores later swells
(e.g. time from 10 % to 50 % of the envelope maximum reached within the first 150 ms) and re-derive the band from
the stored data. The other T-022b criteria (harmonic error, centroid ratio, dynamics ordering) are unaffected.

**Impact.** Only `test_realism_against_tinysol`'s attack assertions. The test file and `tests/FROZEN_MANIFEST.sha256`
were NOT modified. Implementation continues; T-022b stays red until the human approves.
