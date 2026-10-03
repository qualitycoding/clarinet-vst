# Third-party notices

The source code of this repository is licensed under the [Apache License 2.0](LICENSE). This file lists everything
else that this project builds on, uses at build/test time, or is based on. "Shipped" means included in a built plugin.

## Libraries used by the plugin

| Component | Licence | Use | Shipped |
|---|---|---|---|
| [JUCE](https://github.com/juce-framework/JUCE) 9.0.3 | JUCE modules are dual-licensed: AGPLv3 or the commercial JUCE licence | plugin framework, GUI, VST3/AU/Standalone wrappers | yes (statically linked). A plugin binary built with JUCE is subject to the licence you build under: AGPLv3 (the binary and its complete source must be offered under AGPLv3 terms) or a JUCE licence. This repository does not distribute binaries. |
| VST 3 SDK (bundled in JUCE) | MIT (from VST 3.8, October 2025) | VST3 plugin interface | yes |
| [nlohmann/json](https://github.com/nlohmann/json) 3.12.0 | MIT | parsing the resonator table and plugin state | yes |

## Build, test and CI tools (not shipped)

| Component | Licence | Use |
|---|---|---|
| [Catch2](https://github.com/catchorg/Catch2) 3.16.0 | BSL-1.0 | C++ tests |
| [pluginval](https://github.com/Tracktion/pluginval) 1.0.4 | GPLv3 | plugin validation in CI (downloaded and run, never linked) |
| numpy, scipy, soundfile, pytest, pip-audit | BSD / BSD / BSD / MIT / Apache-2.0 | offline Python tools and tests (versions pinned in `tools/requirements.txt`) |
| [OpenWInD](https://gitlab.inria.fr/openwind/openwind) | GPLv3 | optional manual cross-check only; never imported by committed code |

## Reference data (not distributed, not shipped)

| Dataset | Licence | Use |
|---|---|---|
| [TinySOL](https://zenodo.org/record/3685367) v6.0 (Cella et al.), clarinet in B-flat recordings | CC BY 4.0 | downloaded at test time for the objective realism comparison (T-022b). Attribution: C. E. Cella, D. Ghisi, V. Lostanlen, F. Lévy, J. Fineberg, Y. Maresz, "OrchideaSOL: a dataset of extended instrumental techniques for computer-aided orchestration", ICMC 2020. |

## Published work the sound model is based on

The model uses equations, parameter values and geometries from the following papers; no code or text was copied.

- E. A. Petersen, T. Colinot, P. Guillemain, J. Kergomard, "The link between the tonehole lattice cutoff frequency
  and clarinet sound radiation: a quantitative study", Acta Acustica 4 (2020) 18 (CC BY 4.0) — clarinet reed
  model, reed and control parameters, resonator geometry.
- T. Colinot, C. Vergez, P. Guillemain, J.-B. Doc, "Multistability of saxophone oscillation regimes and its
  influence on sound synthesis", Acta Acustica 5 (2021) 33 (CC BY 4.0) — modal time-domain scheme, used for code
  verification.
- V. Debut, J. Kergomard, F. Laloë, "Analysis and optimisation of the tuning of the twelfths for a clarinet
  resonator", Applied Acoustics 66 (2005) 365 — register-hole and bore dimensions.
- N. Szwarcberg et al., work on second-register production and register jumps on the clarinet (JASA 156 (2024) 726;
  arXiv 2506.03875; arXiv 2601.01981) — evidence that register-hole nonlinear losses are needed for the clarion register.

## Facts used from a reference

- Standard Boehm-system B-flat clarinet fingerings come from the Woodwind Fingering Guide (wfg.woodwind.org); only the
  facts (which keys are pressed for which note) are used, as a fixture in `tests/fixtures/`.
