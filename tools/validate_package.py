#!/usr/bin/env python3
import json
import pathlib
import stat
import sys
import tarfile


def fail(message: str) -> None:
    raise SystemExit(f"invalid package: {message}")


def main() -> None:
    if len(sys.argv) != 2:
        fail("usage: validate_package.py PACKAGE.tar.gz")
    package = pathlib.Path(sys.argv[1])
    with tarfile.open(package, "r:gz") as archive:
        members = archive.getmembers()
        names = {member.name for member in members}
        roots = {name.split("/", 1)[0] for name in names}
        if roots != {"rhodes-daily-terminal"}:
            fail("archive must contain one rhodes-daily-terminal root directory")
        config_name = "rhodes-daily-terminal/appconfig.json"
        if config_name not in names:
            fail("appconfig.json is missing")
        config_file = archive.extractfile(config_name)
        if config_file is None:
            fail("appconfig.json is not a regular file")
        config = json.load(config_file)
        required = {"version", "app_ver", "uuid", "type", "screens", "executable"}
        missing = required.difference(config)
        if missing:
            fail(f"appconfig.json missing fields: {sorted(missing)}")
        if config["version"] != 1 or config["type"] != "fg":
            fail("unsupported manifest version or application type")
        if "360x640" not in config["screens"]:
            fail("360x640 screen declaration is required")
        executable = config["executable"]
        executable = executable.get("file") if isinstance(executable, dict) else executable
        executable_name = f"rhodes-daily-terminal/{executable}"
        if executable_name not in names:
            fail(f"declared executable is missing: {executable}")
        info = archive.getmember(executable_name)
        if not info.isfile() or not info.mode & stat.S_IXUSR:
            fail("declared executable must be a user-executable regular file")
    print(f"package valid: {package}")


if __name__ == "__main__":
    main()
