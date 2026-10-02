# Question tree (software profile only)

Each leaf names what it informs. Status after round 3: all answered (see research/claims.json).

## Engineering
- E1 Framework/licence: current JUCE release, licence of modules and VST3 SDK → D-001, D-017 (C-001..C-003)
- E2 Pinned dependency SHAs (Catch2, nlohmann/json, pluginval, GitHub Actions) → D-002, D-020 (C-018, C-020)
- E3 Headless plugin validation on Linux → T-023 (C-019)
- E4 Python toolchain pins → ENVIRONMENT (C-023)
- E5 Reference audio: dataset, licence, metadata columns, clarinet naming, pitch convention → D-015, D-021 (C-005)

## Acoustics / DSP (load-bearing for "totally realistic" and the overblow setting)
- A1 Which exciter model and parameters are published for the clarinet? → D-005, D-011, T-007 (C-006, C-026, C-007, C-021)
- A2 What resonator geometry gives clarinet character (bore, lattice cutoff, register hole)? → D-009, T-030, T-032 (C-016, C-025, C-028, C-030, C-031)
- A3 Does a linear modal model produce the clarion (twelfth) with the register hole open? → D-009 emulation, T-009, T-012 (C-027, C-035)
  - A3.1 If not, what emulation works and is robust to attack/dynamics? → spikes C3, C4, C9
  - A3.2 Register changes while sounding (legato)? → D-011, T-012 legato case (C-004)
- A4 How does a clarinet overblow without the register key; what model change reproduces it? → D-006, D-007, T-013 (C-033)
- A5 How to model the altissimo above the lattice cutoff? → D-009 altissimo rule (C-032)
- A6 Intonation offsets (playing frequency vs impedance peak) and twelfth tuning → D-008, T-010, T-026 (C-013, C-029)
- A7 Radiated-spectrum features for the output stage → D-014 (C-015)
- A8 Stability limits of control parameters (gamma ceiling) → D-006, D-011 (C-009)
- A9 Squeaks: conditions and avoidance at normal playing → D-011 (C-022)

## Instrument facts (UI)
- I1 Range and transposition → T-001 (C-024)
- I2 Boehm lever set → T-002, T-018 (C-011)
- I3 Standard fingerings for every note → T-003, T-004, G-004 (C-010, C-012, C-034)
