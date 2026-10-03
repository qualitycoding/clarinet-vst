#!/usr/bin/env bash
# FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
# SPDX-License-Identifier: Apache-2.0
# T-028: fails if any frozen artifact changed. Run from the repository root.
set -euo pipefail
sha256sum --check --strict --quiet tests/FROZEN_MANIFEST.sha256
echo "freeze OK"
