# Deviations log

| Step | Situation | Choice | Rationale |
|---|---|---|---|
| S-005 | T-009 reads `resonators_valid.json` through `ResonatorTable::fromJson` (S-007), so it cannot run at S-005. | T-007/T-008 verified at S-005; T-009 verified at S-007. | Ordering inside the plan; no code or test change. |
| S-008 | `ClarinetVoice::latencySamples()` added to the public header (D-005 requires reporting the decimator latency to the host; no accessor existed). | Additive method, no existing signature changed. | The plugin needs the value for `setLatencySamples`. |
| S-008 | D-011 gives `zeta = 0.40 - 0.15*reedHardness` (0.325 at the default hardness 0.5). All register spikes (C4/C8/C9) and frozen T-009 used zeta = 0.4. | `zeta = 0.40 + 0.15*(0.5 - hardness)` clamped to [0.25, 0.45]: 0.40 at the default. | Keeps the spike-verified operating point at the default setting; S-016 lists the zeta map as tunable. |
| S-005/S-008 | `simulateColinot` first scaled the whole pole by `tuningScale`. | Scales Im(s) only (identical at scale 1). | D-008 specifies Im(s) scaling. |
