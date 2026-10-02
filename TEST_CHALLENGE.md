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
