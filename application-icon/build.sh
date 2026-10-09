#!/bin/sh
# Regenerate the application icon PNGs and pioneer.ico from the Casimir logo SVGs.
# Needs Inkscape 1.x, optipng and ImageMagick.
# casimir-logo.svg is used at 128 px and up; casimir-logo-small.svg (no waves or stars) below that.
# The in-game copies live in data/icons/: logo.svg (= casimir-logo.svg), badge.png (256 px)
# and badge32-8b.png (32 px, 8-bit palette); refresh them by hand after changing the SVGs.

set -e
cd "$(dirname "$0")"

build_png() {
   SIZE=$1
   SVG=$2
   OUTFILE="pngs/pioneer-${SIZE}x${SIZE}.png"
   test "$SVG" -nt "$OUTFILE" || return 0
   printf 'Generating %sx%s PNG from %s\n' "$SIZE" "$SIZE" "$SVG"
   inkscape --export-type=png --export-filename="$OUTFILE" -w "$SIZE" -h "$SIZE" \
      --export-area-page --export-background-opacity=0 "$SVG"
   optipng -quiet -clobber "$OUTFILE"
}

test -d pngs || mkdir pngs
for sz in 256 128; do build_png $sz casimir-logo.svg; done
for sz in 64 48 40 32 24 22 16; do build_png $sz casimir-logo-small.svg; done

convert pngs/pioneer-16x16.png pngs/pioneer-24x24.png pngs/pioneer-32x32.png \
   pngs/pioneer-48x48.png pngs/pioneer-64x64.png pngs/pioneer-256x256.png pioneer.ico
