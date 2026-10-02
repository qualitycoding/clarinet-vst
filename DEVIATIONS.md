# Deviations log

| Step | Situation | Choice | Rationale |
|---|---|---|---|
| S-005 | T-009 reads `resonators_valid.json` through `ResonatorTable::fromJson` (S-007), so it cannot run at S-005. | T-007/T-008 verified at S-005; T-009 verified at S-007. | Ordering inside the plan; no code or test change. |
