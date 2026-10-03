# SPDX-License-Identifier: Apache-2.0
"""TinySOL reference data (Zenodo record 3685367, CC BY 4.0) — download and B-flat clarinet selection (D-015, D-021)."""
from __future__ import annotations

import csv
import hashlib
import json
import tarfile
import time
import urllib.request
from dataclasses import dataclass
from pathlib import Path

ZENODO_RECORD = "3685367"
ARCHIVE = "TinySOL.tar.gz"
METADATA = "TinySOL_metadata.csv"
REQUIRED_COLUMNS = ("Path", "Family", "Instrument (in full)", "Pitch ID", "Dynamics", "Needed digital retuning")

_API = f"https://zenodo.org/api/records/{ZENODO_RECORD}"
_MARKER = ".tinysol-complete"
_DYNAMICS = ("pp", "mf", "ff")


@dataclass(frozen=True)
class RefNote:
    path: Path        # absolute path to the .wav
    pitch_id: int     # "Pitch ID" as given in the metadata
    dynamics: str     # "pp" | "mf" | "ff"
    retuned: bool     # "Needed digital retuning" flag


def _get(url: str, retries: int = 3, backoff_s: float = 30.0):
    last: Exception | None = None
    for attempt in range(retries):
        try:
            return urllib.request.urlopen(url, timeout=120)
        except Exception as exc:  # network errors are retried, then reported
            last = exc
            if attempt + 1 < retries:
                time.sleep(backoff_s)
    raise RuntimeError(f"cannot fetch {url}: {last}")


def _fetch_file(url: str, target: Path, md5: str) -> None:
    digest = hashlib.md5()
    tmp = target.with_suffix(target.suffix + ".part")
    with _get(url) as resp, open(tmp, "wb") as out:
        while chunk := resp.read(1 << 20):
            digest.update(chunk)
            out.write(chunk)
    if digest.hexdigest() != md5:
        tmp.unlink(missing_ok=True)
        raise RuntimeError(f"md5 mismatch for {target.name}: got {digest.hexdigest()}, Zenodo reports {md5}")
    tmp.replace(target)


def download(dest: Path) -> Path:
    """Fetch archive + metadata from Zenodo into dest (idempotent); verify each file's MD5 against the
    checksum the Zenodo API reports; extract; return dest. Raises RuntimeError on mismatch."""
    dest = Path(dest)
    dest.mkdir(parents=True, exist_ok=True)
    if (dest / _MARKER).exists() and (dest / METADATA).exists():
        return dest
    with _get(_API) as resp:
        record = json.loads(resp.read().decode("utf-8"))
    files = {f["key"]: f for f in record.get("files", [])}
    for name in (METADATA, ARCHIVE):
        if name not in files:
            raise RuntimeError(f"Zenodo record {ZENODO_RECORD} has no file {name}")
        entry = files[name]
        checksum = entry.get("checksum", "")
        if not checksum.startswith("md5:"):
            raise RuntimeError(f"no md5 checksum reported for {name}")
        url = entry.get("links", {}).get("self") or entry.get("links", {}).get("download")
        if not url:
            raise RuntimeError(f"no download link for {name}")
        _fetch_file(url, dest / name, checksum[4:])
    with tarfile.open(dest / ARCHIVE, "r:gz") as tar:
        tar.extractall(dest, filter="data")  # 'data' filter: no absolute paths, links or devices
    (dest / _MARKER).write_text("ok\n")
    return dest


def clarinet_notes(root: Path) -> list[RefNote]:
    """All ordinario rows whose "Instrument (in full)" contains "Clarinet" (TinySOL: "Clarinet in B-flat",
    C-005), dynamics in {pp, mf, ff}, sorted by (pitch_id, dynamics). Raises ValueError if the metadata
    header lacks a column of REQUIRED_COLUMNS or if no clarinet row exists."""
    root = Path(root)
    meta = root / METADATA
    if not meta.exists():
        raise ValueError(f"{meta} not found")
    with open(meta, newline="", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        header = reader.fieldnames or []
        missing = [c for c in REQUIRED_COLUMNS if c not in header]
        if missing:
            raise ValueError(f"metadata lacks required column(s): {missing}")
        rows: list[RefNote] = []
        for r in reader:
            if "clarinet" not in r["Instrument (in full)"].lower():
                continue
            if r["Dynamics"] not in _DYNAMICS:
                continue
            path = (root / r["Path"]).resolve()
            rows.append(RefNote(path=path, pitch_id=int(r["Pitch ID"]), dynamics=r["Dynamics"],
                                retuned=r["Needed digital retuning"].strip().lower() in ("true", "1", "yes")))
    if not rows:
        raise ValueError("no clarinet rows in the metadata")
    rows.sort(key=lambda n: (n.pitch_id, _DYNAMICS.index(n.dynamics), str(n.path)))
    return rows
