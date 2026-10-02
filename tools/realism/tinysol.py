# SPDX-License-Identifier: Apache-2.0
"""TinySOL reference data (Zenodo record 3685367, CC BY 4.0) — download and alto-sax selection.
STUB: raises NotImplementedError."""
from __future__ import annotations
from dataclasses import dataclass
from pathlib import Path

ZENODO_RECORD = "3685367"
ARCHIVE = "TinySOL.tar.gz"
METADATA = "TinySOL_metadata.csv"


@dataclass(frozen=True)
class RefNote:
    path: Path        # absolute path to the .wav
    midi: int         # concert MIDI pitch ("Pitch ID")
    dynamics: str     # "pp" | "mf" | "ff"
    retuned: bool     # "Needed digital retuning" flag


def download(dest: Path) -> Path:
    """Fetch archive + metadata from Zenodo into dest (idempotent); verify each file's MD5 against
    the checksum the Zenodo API reports; extract; return dest. Raises RuntimeError on mismatch."""
    raise NotImplementedError("download")


def alto_notes(root: Path) -> list[RefNote]:
    """All Alto Saxophone ordinario notes (Instrument (in full) == 'Alto Saxophone'), sorted by
    (midi, dynamics). Raises ValueError if the metadata header lacks a required column."""
    raise NotImplementedError("alto_notes")
