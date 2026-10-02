# FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
# SPDX-License-Identifier: Apache-2.0
"""T-024 (security: supply chain & licence compliance) — SC-10; decisions D-002, D-017."""
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def test_cmake_dependencies_pinned_to_full_sha():
    text = (ROOT / "CMakeLists.txt").read_text()
    tags = re.findall(r"GIT_TAG\s+(\S+)", text)
    assert len(tags) >= 3
    for t in tags:
        assert re.fullmatch(r"[0-9a-f]{40}", t), t


def test_python_requirements_pinned():
    for line in (ROOT / "tools/requirements.txt").read_text().splitlines():
        line = line.strip()
        if line and not line.startswith("#"):
            assert re.fullmatch(r"[A-Za-z0-9_.\-]+==[0-9][^ ]*", line), line


def test_ci_actions_pinned_to_full_sha():
    wf = list((ROOT / ".github/workflows").glob("*.yml"))
    assert wf, "CI workflow missing (S-002)"
    for f in wf:
        for use in re.findall(r"uses:\s*([^\s#]+)", f.read_text()):
            assert re.fullmatch(r"[\w.\-]+/[\w.\-/]+@[0-9a-f]{40}", use), (f.name, use)


def test_licence_and_notices_present():
    assert "Apache License" in (ROOT / "LICENSE").read_text()
    notices = (ROOT / "THIRD_PARTY_NOTICES.md").read_text()
    for name in ("JUCE", "AGPL", "VST 3", "MIT", "nlohmann", "Catch2", "TinySOL", "CC BY 4.0", "Colinot"):
        assert name in notices, name
