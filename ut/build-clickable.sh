#!/bin/sh
set -e

cd "$(dirname "$0")/.."

case "$ARCH" in
  arm64) cross=aarch64-linux-gnu- ;;
  armhf) cross=arm-linux-gnueabihf- ;;
  *)
    echo "Unsupported architecture: $ARCH" >&2
    exit 1
    ;;
esac

if [ -z "$INSTALL_DIR" ]; then
  echo "INSTALL_DIR is not set; run this script through clickable" >&2
  exit 1
fi

jobs=$(getconf _NPROCESSORS_ONLN 2>/dev/null || nproc 2>/dev/null || echo 1)

host_arch=$(dpkg --print-architecture 2>/dev/null || true)
case "$host_arch" in
  aarch64) host_arch=arm64 ;;
  armv7l) host_arch=armhf ;;
esac
if command -v "${cross}gcc" >/dev/null 2>&1; then
  cross_arg="CROSS_COMPILE=$cross"
else
  if [ "$host_arch" != "$ARCH" ]; then
    echo "No $ARCH cross compiler found (host: ${host_arch:-unknown})" >&2
    exit 1
  fi
  cross_arg=
fi

make -j"$jobs" -f Makefile.ut click \
  $cross_arg \
  CLICK_ARCH="$ARCH" \
  INSTALL_DIR="$INSTALL_DIR" \
  OBJDIR_BASE="obj-unix/$ARCH_TRIPLET" \
  TARGET="retroarch-$ARCH"

if [ ! -f "$INSTALL_DIR/manifest.json" ]; then
  echo "Manifest missing from $INSTALL_DIR" >&2
  exit 1
fi
