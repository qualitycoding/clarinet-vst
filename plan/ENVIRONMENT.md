# Environment (pinned; verified in the planning sandbox on 2026-10-02)

## Toolchain
| Component | Version / pin | Notes |
|---|---|---|
| OS (verified) | Ubuntu 24.04 x86-64 (1 vCPU, 3 GB RAM) | CI also macos-15, windows-2025 (A-018) |
| C++ compiler | GCC 13.3.0 (verified); Apple Clang (Xcode default on macos-15); MSVC 2022 | C++20 |
| CMake | 4.3.4 (`pip install cmake==4.3.4`, verified); any ≥ 3.22 | |
| Ninja | 1.13.x (`pip install ninja`) | Windows: VS generator acceptable |
| JUCE | 9.0.3 @ `be29c81492b6151c8ea8d14c840e1311963b3a83` | FetchContent (C-002) |
| Catch2 | v3.16.0 @ `317ac1ed4c0bb6e6b91eafc817e05c488feffcb3` | tests only (C-018) |
| nlohmann/json | v3.12.0 @ `55f93686c01528224f448c19128836e7df245f72` | core, private (C-018) |
| pluginval | v1.0.4, zip SHA-256s in `tests/scripts/run_pluginval.sh` | CI/test only (C-018, C-019) |
| Python | 3.12.3 | tools + Python tests |
| Python packages | `tools/requirements.txt` (numpy 2.5.3, scipy 1.18.1, soundfile 0.14.0, pytest 9.1.1, pip-audit 2.10.1); lock `tools/requirements.lock` | verified install (C-023) |

## Linux system packages (verified install)
```bash
sudo apt-get update   # an unrelated broken nodesource apt source may need disabling first
sudo apt-get install -y g++ libasound2-dev libjack-jackd2-dev ladspa-sdk libfreetype-dev \
  libfontconfig1-dev libx11-dev libxcomposite-dev libxcursor-dev libxext-dev libxinerama-dev \
  libxrandr-dev libxrender-dev libxi-dev libcurl4-openssl-dev xvfb
```
(`JUCE_WEB_BROWSER=0`, `JUCE_USE_CURL=0`.)

## Python tools (verified)
```bash
python3 -m venv .venv && . .venv/bin/activate
pip install -r tools/requirements.txt
```

## Build commands (verified)
```bash
pip install cmake==4.3.4 ninja           # if no system CMake >= 3.22
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release            # plugin ON by default
cmake --build build -j1                                            # -j1 on <= 4 GB RAM (LTO link)
ctest --test-dir build -LE perf --output-on-failure
ctest --test-dir build -L perf --output-on-failure                 # Release only (T-016)
python -m pytest tests/python -k "not tinysol"
bash tests/scripts/verify_freeze.sh
```
Core-only configure: `-DCLAR_BUILD_PLUGIN=OFF`. Faster local configure (optional, used in planning): pass
`-DFETCHCONTENT_SOURCE_DIR_JUCE=<dir>` pointing at a shallow fetch of the pinned JUCE commit
(`git fetch --depth 1 origin be29c81…`); the pin is unchanged.

## Verification log (planning sandbox)
- Debug core + tests (`-DCLAR_BUILD_PLUGIN=OFF`): configure + build OK (142 targets). Red run: unit 18/18
  cases fail, integration 17/17 fail, operational 4/4, alloc 1/1; perf skipped in Debug (fails at voice
  construction in Release).
- Python venv from pins: OK. Red run: 24 fail / 2 pass (T-024a, T-024b guard tests, D-018).
- Release build with plugin and pluginval: see `research/spikes/plugin_build.log`.
