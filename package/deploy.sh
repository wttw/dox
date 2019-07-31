#!/bin/bash

APPNAME=dox
SCRATCH=deploy-scratch

die() {
  echo "${BASH_SOURCE[1]}: line ${BASH_LINENO[0]}: ${FUNCNAME[1]}: ${1-Died}" >&2
  exit 1
}

set -o pipefail -o noclobber -o nounset

if [[ ${BUILDKITE:-false} != "true" ]]
then
    die "Must be run as buildkite job"
else
    rm -rf "${SCRATCH}"
    mkdir "${SCRATCH}" || die "Failed to make a scratch directory"

    VERSION=$(buildkite-agent meta-data get release-version)
    TYPE=$(buildkite-agent meta-data get release-type)
    echo Fetching artifacts for ${VERSION}   
    buildkite-agent artifact download "*" ${SCRATCH} || die "failed to download artifacts"
fi

[[ ${VERSION} =~ ^([0-9]+\.[0-9]+\.[0-9]+)\.[0-9]+$ ]] || die "Invalid version: $VERSION" 

SHORTVERSION="${BASH_REMATCH[1]}"

rm -rf files-for-release
mkdir files-for-release

cd ${SCRATCH}

for file in windows\\* ; do
    [[ $file =~ ^windows\\(.+)$ ]] || die
    mkdir -p windows
    mv "${file}" "windows/${BASH_REMATCH[1]}" || die "Failed to move windows asset"
done

for os in * ; do
    cd "${os}" || die "Failed to cd to ${SCRATCH}/${os}"
    for file in * ; do
	cp "${file}" "../../files-for-release/${os}_${file}" || die "Failed to copy ${file}"
    done
    cd ..
done

cd ..

if [[ $TYPE == "stable" ]]
then
    github-release "v${SHORTVERSION}" files-for-release/* --github-repository wttw/dox --github-access-token ${GITHUB_TOKEN}
else
    github-release "v${SHORTVERSION}" files-for-release/* --github-repository wttw/dox --github-access-token ${GITHUB_TOKEN} --prerelease
fi
rm -rf files-for-release
