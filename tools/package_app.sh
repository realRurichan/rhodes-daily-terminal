#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD_DIR=${BUILD_DIR:-"$ROOT/build-device"}
BINARY=${1:-"$BUILD_DIR/daily_terminal"}
OUT_DIR=${OUT_DIR:-"$ROOT/packages"}

[ -f "$BINARY" ] || {
  echo "missing device binary: $BINARY" >&2
  echo "build with EPASS_NATIVE=ON first, or pass its path as argument" >&2
  exit 1
}

VERSION=$(python3 -c "import json; print(json.load(open('$ROOT/appconfig.json', encoding='utf-8'))['app_ver'])")
STAGE=$(mktemp -d)
trap 'rm -rf "$STAGE"' EXIT HUP INT TERM
APP_DIR="$STAGE/rhodes-daily-terminal"
mkdir -p "$APP_DIR" "$OUT_DIR"
cp "$BINARY" "$APP_DIR/daily_terminal"
cp "$ROOT/appconfig.json" "$APP_DIR/appconfig.json"
cp "$ROOT/icon.png" "$APP_DIR/icon.png"
chmod 755 "$APP_DIR/daily_terminal"

PACKAGE="$OUT_DIR/rhodes-daily-terminal-v$VERSION.tar.gz"
tar -C "$STAGE" -czf "$PACKAGE" rhodes-daily-terminal
python3 "$ROOT/tools/validate_package.py" "$PACKAGE"
echo "package ready: $PACKAGE"
