# SPDX-License-Identifier: Apache-2.0
"""CLI: render every TinySOL clarinet note with clar_render and write results.json (D-015).
Usage: python -m tools.realism.compare_tinysol --render BUILD/tools/render/clar_render --tinysol DIR --out FILE
STUB."""
from __future__ import annotations
import sys

VELOCITY = {"pp": 0.25, "mf": 0.6, "ff": 0.95}  # D-015 dynamic mapping


def main(argv: list[str] | None = None) -> int:
    raise NotImplementedError("compare_tinysol.main")


if __name__ == "__main__":
    sys.exit(main())
