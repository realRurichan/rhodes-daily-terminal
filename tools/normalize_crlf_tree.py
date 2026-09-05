#!/usr/bin/env python3
"""Normalize CRLF in UTF-8 text files without touching binary assets."""

import pathlib
import sys

root = pathlib.Path(sys.argv[1])
changed = 0
for path in root.rglob("*"):
    if not path.is_file() or ".git" in path.parts:
        continue
    data = path.read_bytes()
    if b"\0" in data or b"\r\n" not in data:
        continue
    try:
        data.decode("utf-8")
    except UnicodeDecodeError:
        continue
    path.write_bytes(data.replace(b"\r\n", b"\n"))
    changed += 1
print(f"normalized {changed} UTF-8 text files")
