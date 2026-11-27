#!/usr/bin/env bash
# Build optional controller libraries for libogc2 into ./portlibs/ppc.
set -e

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PFX="$ROOT/portlibs/ppc"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

GCC=/opt/devkitpro/devkitPPC/bin/powerpc-eabi-gcc
AR=/opt/devkitpro/devkitPPC/bin/powerpc-eabi-ar
LIBOGC_INC=/opt/devkitpro/libogc2/wii/include
CF="-O2 -Wall -DGEKKO -mrvl -mcpu=750 -meabi -mhard-float -I$LIBOGC_INC"

git clone --quiet https://github.com/FIX94/libwiidrc.git "$WORK/libwiidrc"
git -C "$WORK/libwiidrc" checkout --quiet 4e156201db17231ec6664d66e478151a859f8c5b
git clone --quiet https://github.com/FIX94/libwupc.git "$WORK/libwupc"
git -C "$WORK/libwupc" checkout --quiet 24145b35dbc2c1950f6f48079d577acc7efb0c4d
git clone --quiet https://github.com/xerpi/libsicksaxis.git "$WORK/libsicksaxis"
git -C "$WORK/libsicksaxis" checkout --quiet 4ddd49e687a7d2466691fd2c73d8a55a90fe1ca6

mkdir -p "$PFX/lib" "$PFX/include/wiidrc" "$PFX/include/wupc"

$GCC $CF -I"$WORK/libwiidrc/include" -I"$WORK/libwiidrc/source" \
    -c "$WORK/libwiidrc/source/wiidrc.c" -o "$WORK/wiidrc.o"
$AR rcs "$PFX/lib/libwiidrc.a" "$WORK/wiidrc.o"
cp "$WORK/libwiidrc/include/wiidrc/wiidrc.h" "$PFX/include/wiidrc/"

$GCC $CF -I"$WORK/libwupc/include" -I"$WORK/libwupc/source" \
    -c "$WORK/libwupc/source/wupc.c" -o "$WORK/wupc.o"
$AR rcs "$PFX/lib/libwupc.a" "$WORK/wupc.o"
cp "$WORK/libwupc/include/wupc/wupc.h" "$PFX/include/wupc/"

$GCC $CF -c "$WORK/libsicksaxis/libsicksaxis/sicksaxis.c" \
    -o "$WORK/sicksaxis.o"
$AR rcs "$PFX/lib/libsicksaxis.a" "$WORK/sicksaxis.o"
cp "$WORK/libsicksaxis/libsicksaxis/sicksaxis.h" "$PFX/include/"

echo "Built optional controller libraries in $PFX"
