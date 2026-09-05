#!/usr/bin/env python3
import json
import pathlib
import sys
import uuid

path = pathlib.Path(sys.argv[1])
data = json.loads(path.read_text(encoding="utf-8"))
assert data["version"] == 1
assert isinstance(data["app_ver"], int) and data["app_ver"] > 0
assert data["type"] == "fg"
assert "360x640" in data["screens"]
assert data["executable"]["file"] == "daily_terminal"
uuid.UUID(data["uuid"])
print("manifest valid")
