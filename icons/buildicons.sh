#!/bin/bash

# Create icons for windows and mac.

# Run at generate time to avoid the need for assorted image
# conversion tools to be acquired by people wanting to just
# check out and build.

# Requires rsvg-convert and icotool installed:
# apt install librsvg2-bin icoutils

NAME=dox

die() {
  echo "${BASH_SOURCE[1]}: line ${BASH_LINENO[0]}: ${FUNCNAME[1]}: ${1-Died}" >&2
  exit 1
}

BASEDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd "$BASEDIR"

[[ -f "${NAME}.svg" ]] || die "svg icon source ${NAME}.svg not found"

for width in 16 24 32 48 64 128 256
do
    rsvg-convert --width=$width --height=$width --output="icon_${width}x${width}.png" "${NAME}.svg" || die "failed to render svg"
done

ARGS=(--create --output=dox.ico -r icon_256x256.png)

for width in 16 24 32 48
do
    ARGS+=("icon_${width}x${width}.png")
done	     

# Apparently the large, 256x256, icon needs to also be included as the raw png
icotool ${ARGS[@]} || die "Failed to create dox.ico"

