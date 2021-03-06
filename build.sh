#!/bin/bash

# Build script, to be run under wsl on Windows and bash elsewhere

APPNAME=dox
DESCRIPTION="${APPNAME} dns tool"
# Error handling

die() {
  echo "${BASH_SOURCE[1]}: line ${BASH_LINENO[0]}: ${FUNCNAME[1]}: ${1-Died}" >&2
  exit 1
}

# Be more defensive
set -o pipefail -o noclobber -o nounset

# macOS has old-ish versions of bash, so we can't use modern getopt
PARSED=$(getopt dh:t: $*)
if [[ $? -ne 0 ]]; then
    # e.g. return value is 1
    #  then getopt has complained about wrong arguments to stdout
    echo "Usage: build.sh -d -h hostos -t targetdistro" 1>&2
    exit 2
fi

eval set -- $PARSED

FLAG_DEBUG=0
HOST=unknown
TARGET=unknown

for i; do
  case "$i" in
    -d)
      FLAG_DEBUG=1
      shift
      ;;
    -h)
      HOST="$2"
      shift
      shift
      ;;
    -t)
      TARGET="$2"
      shift
      shift
      ;;
    --)
      shift
      break
      ;;
    *)
      die "Unexpected option: $1"
      ;;
  esac
done

if [[ "$HOST" == "unknown" ]] ; then
  case "$(uname -s)" in
    Darwin)
      HOST=darwin
      DISTRO=darwin
      ;;
    Linux)
      if grep -qE "(Microsoft|WSL)" /proc/version &> /dev/null ; then
        HOST=windows
	DISTRO=windows
      else
        [[ -f /etc/os-release ]] || die "No /etc/os-release"
	# If we can't trust /etc/os-release not to be hostile ...
	. /etc/os-release
        case ${ID} in
          ubuntu)
            HOST=ubuntu
	    DISTRO="${VERSION_CODENAME}"
            ;;
          fedora)
            HOST=fedora
	    DISTRO="${ID}${VERSION_ID}"
            ;;
          *)
            die "Unexpected OS ${RELEASE}"
            ;;
        esac
      fi
      ;;
    *)
      die "Unhandled operating system name: $(uname -s)"
      ;;
  esac
fi

if [[ "$TARGET" == "unknown" ]] ; then
    TARGET="${DISTRO}"
fi

# On wsl under windows we can call windows binaries by including the
# .exe extension. Without it we get the linux binaries, which we mostly
# don't want
EXE=""

case "$HOST" in
  darwin)
    ;;
  ubuntu)
    ;;
  fedora)
    ;;
  windows)
    EXE=".exe"
    ;;
  *)
    die "Unhandled host: $HOST"
    ;;
esac

CMAKE=cmake${EXE}
NINJA=ninja${EXE}
QTPATHS=qtpaths${EXE}
GIT=git${EXE}
BKAGENT=
TOOLS=("$CMAKE" "$NINJA" "$QTPATHS" "$GIT")

if [[ ${BUILDKITE:-false} == "true" ]]
then
BKAGENT=buildkite-agent${EXE}
TOOLS+=("$BKAGENT")
fi

# Sanity check our environment to fail early
for i in "${TOOLS[@]}" ; do
  hash "$i" 2>/dev/null || die "No $i found"
done

# In release build set VERSION from buildkite
if [[ ${BUILDKITE:-false} == "true" ]]
then
    VERSION=$(${BKAGENT} meta-data get release-version || true)
fi

# If VERSION hasn't been set, sniff it from git tags
if [[ -z "${VERSION:-}" ]]
then
GITTAG=$("${GIT}" describe --abbrev=0 --match 'v*')

[[ ${GITTAG} =~ ^v([0-9]+\.[0-9]+\.[0-9]+)[a-z]? ]] || die "Invalid git tag: $GITTAG"

GITSHORTVERSION="${BASH_REMATCH[1]}"

VERSION="${GITSHORTVERSION}.${BUILDKITE_BUILD_NUMBER:-0}"
fi


[[ ${VERSION} =~ ^([0-9]+\.[0-9]+\.[0-9]+)\.[0-9]+$ ]] || die "Invalid version: $VERSION" 

SHORTVERSION="${BASH_REMATCH[1]}"


# The base directory of the checkout is where this script lives
BASEDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd "$BASEDIR" || die "$BASEDIR is strange"

[[ -d .git ]] || die "$BASEDIR isn't the root of a git checkout"

case "$FLAG_DEBUG" in
  1)
      BUILDTYPE="Debug"
      ;;
  0)
      BUILDTYPE="Release"
      ;;
  *)
    die "Bad debug setting"
    ;;
esac

echo "Building $VERSION on $HOST for $TARGET in $BUILDTYPE mode"

[[ -d build ]] || mkdir build

cd build || die "No build directory"

[[ -d "$BUILDTYPE" ]] || mkdir "$BUILDTYPE"

cd "$BUILDTYPE" || die "No build type directory: '$BUILDTYPE'"

if [[ "$BUILDTYPE" == "Release" ]]
then
    # Signing and packaging can be destructive, linking is cheap
    rm -f "${APPNAME}" "${APPNAME}.exe"
    rm -rf "${APPNAME}.app"
fi

if [[ ! -f build.ninja || ! -f rules.ninja ]]
then
    "$CMAKE" -DCMAKE_BUILD_TYPE=$BUILDTYPE -G Ninja ../.. || die "cmake failed"
fi

# Cmake+Qt do some things a cmake run time they should really do during the build
if [[ $DISTRO == "darwin" && ! -f ${APPNAME}.app/Contents/Info.plist ]]
then
    "$CMAKE" -DCMAKE_BUILD_TYPE=$BUILDTYPE -G Ninja ../.. || die "cmake failed"
fi

"$NINJA" || die "ninja failed"

#export BUILDTYPE BASEDIR HOST DISTRO TARGET VERSION SHORTVERSION

if [[ "$BUILDTYPE" == "Release" ]]
then
  OUTPUTDIR="${BASEDIR}/Output/${VERSION}/${TARGET}"
  mkdir -p "${OUTPUTDIR}" || die "Failed to create outputdir '$OUTPUTDIR'"
  echo "Packaging to ${OUTPUTDIR}" 1>&2
  if [[ -f "${BASEDIR}/package/package-${TARGET}.sh" ]]
  then
    source "${BASEDIR}/package/package-${TARGET}.sh"
  else
    if [[ -f "${BASEDIR}/package/package-for-${HOST}.sh" ]]
    then
      source "${BASEDIR}/package/package-for-${HOST}.sh"
    else
      die "No packaging script for ${HOST} or ${TARGET}"
    fi
  fi
else
  echo "Not packaging in ${BUILDTYPE} build" 1>&2
fi
