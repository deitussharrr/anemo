#!/bin/sh
set -eu

ANEMO=${1:-./anemo}
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
TMP_DIR=$(mktemp -d "${TMPDIR:-/tmp}/anemo-smoke.XXXXXX")
trap 'rm -rf "$TMP_DIR"' EXIT

targets=$("$ANEMO" targets)
printf '%s\n' "$targets" | grep -F "android-arm64" >/dev/null
printf '%s\n' "$targets" | grep -F "linux-arm64" >/dev/null
printf '%s\n' "$targets" | grep -F "linux-x86_64" >/dev/null
printf '%s\n' "$targets" | grep -F "windows-x86_64" >/dev/null
printf '%s\n' "$targets" | grep -F "macos-arm64" >/dev/null

"$ANEMO" build "$ROOT/examples/hello.anm" >/dev/null
binary="$ROOT/examples/hello"
trap 'rm -rf "$TMP_DIR" "$binary" "$ROOT/examples/hello.s" "$ROOT/examples/hello.o"' EXIT

output=$("$binary")
printf '%s\n' "$output" | grep -F "anemo says hello" >/dev/null

version=$("$ANEMO" version)
printf '%s\n' "$version" | grep -F "anemo 0.2.0" >/dev/null

printf '%s\n' "Anemo smoke checks passed."
