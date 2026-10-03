#!/usr/bin/env bash
# FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
# SPDX-License-Identifier: Apache-2.0
# T-023 (operational/integration): validate the built VST3 with pluginval 1.0.4 at strictness 10.
# Usage: tests/scripts/run_pluginval.sh <path/to/Saxophone.vst3>
# Fails (exit != 0) if the bundle is missing, the download checksum mismatches, or validation fails.
set -euo pipefail
BUNDLE="${1:?usage: run_pluginval.sh <Saxophone.vst3>}"
[ -e "$BUNDLE" ] || { echo "T-023 FAIL: plugin bundle not found: $BUNDLE" >&2; exit 1; }
case "$(uname -s)" in
  Linux)  OS=Linux;   SHA=c01c49d8063965c4c2dea8324468336768f5c9139e0b1caebde14c2400b55352; EXE=pluginval ;;
  Darwin) OS=macOS;   SHA=3c4c533bda0c5059eea3ddaea752d757ee2025041f0f47e6bcb0e87f6082b29f; EXE=pluginval.app/Contents/MacOS/pluginval ;;
  MINGW*|MSYS*|CYGWIN*) OS=Windows; SHA=c08e61ce3b96db41636f8ec7e76f4c7e2c13ebdac7fa1b5a1f52b4f32ec715ab; EXE=pluginval.exe ;;
  *) echo "unsupported OS" >&2; exit 1 ;;
esac
WORK="${RUNNER_TEMP:-/tmp}/pluginval-1.0.4"
mkdir -p "$WORK"
ZIP="$WORK/pluginval_$OS.zip"
[ -f "$ZIP" ] || curl -sSfL -o "$ZIP" "https://github.com/Tracktion/pluginval/releases/download/v1.0.4/pluginval_$OS.zip"
if command -v sha256sum >/dev/null; then GOT=$(sha256sum "$ZIP" | cut -d' ' -f1); else GOT=$(shasum -a 256 "$ZIP" | cut -d' ' -f1); fi
[ "$GOT" = "$SHA" ] || { echo "T-023 FAIL: pluginval checksum mismatch ($GOT)" >&2; exit 1; }
unzip -o -q "$ZIP" -d "$WORK"
"$WORK/$EXE" --strictness-level 10 --validate-in-process --timeout-ms 600000 --validate "$BUNDLE"
