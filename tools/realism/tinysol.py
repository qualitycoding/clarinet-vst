# SPDX-License-Identifier: Apache-2.0
"""TinySOL reference data (Zenodo record 3685367, CC BY 4.0) — download and B-flat clarinet selection.
STUB: raises NotImplementedError."""
from __future__ import annotations
from dataclasses import dataclass
from pathlib import Path

ZENODO_RECORD = "3685367"
ARCHIVE = "TinySOL.tar.gz"
METADATA = "TinySOL_metadata.csv"
REQUIRED_COLUMNS = ("Path", "Family", "Instrument (in full)", "Pitch ID", "Dynamics", "Needed digital retuning")


@dataclass(frozen=True)
class RefNote:
    path: Path        # absolute path to the .wav
    pitch_id: int     # "Pitch ID" as given in the metadata
    dynamics: str     # "pp" | "mf" | "ff"
    retuned: bool     # "Needed digital retuning" flag


def download(dest: Path) -> Path:
    """Fetch archive + metadata from Zenodo into dest (idempotent); verify each file's MD5 against the
    checksum the Zenodo API reports; extract; return dest. Raises RuntimeError on mismatch."""
    raise NotImplementedError("download")


def clarinet_notes(root: Path) -> list[RefNote]:
    """All ordinario rows whose "Instrument (in full)" contains "Clarinet" (TinySOL: "Clarinet in B-flat",
    C-005), dynamics in {pp, mf, ff}, sorted by (pitch_id, dynamics). Raises ValueError if the metadata
    header lacks a column of REQUIRED_COLUMNS or if no clarinet row exists."""
    raise NotImplementedError("clarinet_notes")
