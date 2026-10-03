# G-003 objective metrics (T-022b procedure, frozen thresholds in tests/python/test_realism_vs_tinysol.py)

| dynamic | notes | harmonic MAD mean (<= 6) | max (<= 10) | centroid ratio in [0.8,1.25] (>= 90 %) | attack ratio in [0.5,2] (>= 90 %) |
|---|---|---|---|---|---|
| pp | 42 | 9.7 | 26.6 | 95 % | 12 % |
| mf | 42 | 8.9 | 25.4 | 50 % | 0 % |
| ff | 42 | 9.3 | 16.9 | 55 % | 0 % |

Dynamics ordering (our centroid pp < mf < ff): 93 % of 42 pitches (needs >= 90 %).
Odd-harmonic fingerprint, concert D3-B3 pp: median (H3 - H2) real 15.1 dB, ours 12.7 dB.
Odd-harmonic fingerprint, concert D3-B3 mf: median (H3 - H2) real 17.0 dB, ours 21.4 dB.
Odd-harmonic fingerprint, concert D3-B3 ff: median (H3 - H2) real 28.6 dB, ours 20.8 dB.

Attack ratio: see TEST_CHALLENGE.md (the frozen criterion cannot be met by any plausible model).
