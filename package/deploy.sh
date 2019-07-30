#!/bin/bash

APPNAME=dox

die() {
  echo "${BASH_SOURCE[1]}: line ${BASH_LINENO[0]}: ${FUNCNAME[1]}: ${1-Died}" >&2
  exit 1
}

set -o pipefail -o noclobber -o nounset

[[ ${BUILDKITE:-false} == "true" ]] || die "Must be run as buildkite job"

VERSION=$(buildkite-agent meta-data get release-version)


[[ ${VERSION} =~ ^([0-9]+\.[0-9]+\.[0-9]+)\.[0-9]+$ ]] || die "Invalid version: $VERSION" 

SHORTVERSION="${BASH_REMATCH[1]}"

echo Fetching artifacts for ${VERSION}

rm -rf scratch
mkdir scratch || die "Failed to make a scratch directory"

buildkite-agent artifact download "*" scratch || die failed to download artifacts

echo downloaded so many things

