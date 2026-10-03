# Sources (with verification notes)

Tier per protocol R2. "Fetched" = full text or the cited table read on 2026-10-02 unless noted.

| # | Source | Tier | Used for | Verification note |
|---|---|---|---|---|
| S1 | E. A. Petersen, T. Colinot, P. Guillemain, J. Kergomard, "The link between the tonehole lattice cutoff frequency and clarinet sound radiation: a quantitative study", Acta Acustica 4 (2020) 18, doi:10.1051/aacus/2020018, CC BY 4.0 | 1 | reed model eqs (7)–(9), Table 1 geometry, Table 2 reed params, Table 3 controls, radiation features | Fetched HTML incl. Tables 1–3; Table 1 reproduced by spike C10 |
| S2 | T. Colinot, C. Vergez, P. Guillemain, J.-B. Doc, "Multistability of saxophone oscillation regimes…", Acta Acustica 5 (2021) 33, CC BY 4.0 | 1 | integrator scheme, Fs, Table 2 used for code verification (T-008) | Fetched 2026-10-01 (sibling plan), reproduced by sibling spike S1 |
| S3 | N. Szwarcberg, T. Colinot, C. Vergez, M. Jousserand, "Second register production on the clarinet: nonlinear losses in the register hole as the decisive physical phenomenon", JASA 156(2):726–739 (2024); arXiv 2404.07540 | 1 | C-027 | Abstract/introduction read (arXiv HTML) |
| S4 | N. Szwarcberg et al., "Register jumps on the clarinet: numerical and in-vitro investigation into basins of attraction and phase-tipping", arXiv 2506.03875 (2025) | 2 | C-004, C-007, C-009, C-028, C-031 | Full text read |
| S5 | N. Szwarcberg et al., "How localized nonlinear losses condition the acoustical design of a self-sustained oscillator: the clarinet and its register hole", arXiv 2601.01981 (2026) | 2 | C-027 | Abstract read |
| S6 | V. Debut, J. Kergomard, F. Laloë, "Analysis and optimisation of the tuning of the twelfths for a clarinet resonator", Applied Acoustics 66 (2005) 365–409; arXiv physics/0309051 | 1 | register-hole geometry, twelfth tuning (C-028, C-029) | arXiv HTML excerpts read (secs. 3.3, 4.3, 5) |
| S7 | E. Moers, J. Kergomard, "On the cutoff frequency of clarinet-like instruments…", Acta Acust. united Ac. 97 (2011) 984 | 1 | C-016 | Abstract only (via secondary citation in S8) |
| S8 | E. Petersen, T. Colinot, J. Kergomard, P. Guillemain, "On the tonehole lattice cutoff frequency of conical resonators…", Acta Acustica 4 (2020) 13 | 1 | C-016 (clarinet cutoffs 1150–1450 Hz) | Excerpt read |
| S9 | Woodwind Fingering Guide (T. Reichard), Boehm clarinet: fingering scheme, basic charts (chalumeau, clarion, lower altissimo), alternate upper-altissimo chart | 3 | C-010, C-011, C-012, C-024, C-034; fixture | All four pages fetched; single source for the full chart → G-004 |
| S10 | TinySOL v6.0, Zenodo record 3685367 (Cella et al.), CC BY 4.0; metadata CSV | 1 | C-005, D-015 | Record page and CSV header fetched |
| S11 | mirdata TinySOL loader documentation | 3 | instrument list | Read |
| S12 | GitHub REST API (JUCE, Catch2, nlohmann/json, actions/*, pluginval) | 1 | C-002, C-018, C-020 | Queried 2026-10-02 with authenticated API |
| S13 | Sibling plan qualitycoding/saxophone-vst, branch gen-20261001T211329Z-saxophone-vst-plan (claims C-001, C-003, C-008, C-017, C-019, C-023 and test/infra patterns) | 1 | infrastructure carried over | Cloned and read 2026-10-02 |
| S14 | Planning spikes C1–C11 in research/spikes/ (this plan) | 1 (empirical) | C-013, C-022, C-025..C-035 | Code + outputs committed |
