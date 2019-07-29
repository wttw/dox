# -*-bash-*-

# We're called with BUILDTYPE, BASEDIR, OUTPUTDIR, APPNAME, VERSION, SHORTVERSION
# and cwd $BASEDIR/build/$BUILDTYPE

# Make sure we're in the right directory and the app was built
[[ -f "${APPNAME}.app/Contents/MacOS/${APPNAME}" ]] || die "${APPNAME} wasn't built"

function lockup {
  if [[ ! -z "${SIGNKCFILE:-}" ]]
  then
    security lock-keychain "${SIGNKCFILE:-}"
  fi
}

SCRATCH="scratch/Applications"
rm -rf "${SCRATCH}"
mkdir -p "${SCRATCH}" || die "Failed to create scratch directory"
cp -a "${APPNAME}.app" "${SCRATCH}/${APPNAME}.app" || die "Failed to copy app"

cd "${SCRATCH}" || die "Scratch dir went away"

macdeployqt "${APPNAME}.app" -appstore-compliant || die "macdeployqt failed"

# Sign one or more files. No-op if we don't have a certificate configured.
function signfile {
  if [[ -z "${SIGNCERTID:-}" ]]
  then
    return 0
  fi
  TIMESTAMPSERVERS="http://timestamp.comodoca.com/authenticode http://timestamp.verisign.com/scripts/timestamp.dll http://timestamp.globalsign.com/scripts/timestamp.dll http://tsa.starfieldtech.com"
  
  for attempt in {1..120}
  do
# Sign with a certificate SIGNCERTID from a keychain SIGNKCFILE requesting hardened runtime
    if codesign --sign="${SIGNCERTID}" --keychain="${SIGNKCFILE}" --options=runtime $* ; then
      echo "Signed $*" 1>&2
      return 0
    fi
    echo "Failed to sign $*" 1>&2
    sleep 1
  done
  die "Failed to sign $file after many attempts"
}

if [[ ! -z "${SIGNKCFILE:-}" && ! -z "${SIGNKCPASS:-}" && ! -z "${SIGNCERTID:-}" ]]
then
  security unlock-keychain -p "${SIGNKCPASS}" "${SIGNKCFILE}" || die "Failed to unlock keychain"
  trap lockup EXIT
fi 

# Apple discourage use of codesign --deep, so we'll do it the hard way
signfile `find ${APPNAME}.app/Contents/Plugins -type f -and -perm +111 -and -name '*.dylib'`
signfile `find ${APPNAME}.app/Contents/Frameworks -type d -and -name '*.framework'`
signfile "${APPNAME}.app"


#"${BASEDIR}/package/create-dmg" --window-size 480 540 --icon-size 48 --volname ${APPNAME}-${SHORTVERSION} --app-drop-link 240 385 --icon "${APPNAME}" 240 141 "${OUTPUTDIR}/${APPNAME}-${SHORTVERSION}.dmg" "${APPNAME}.app" || die "Failed to create dmg"

#if [[ ! -z "${SIGNKCFILE:-}" && ! -z "${SIGNKCPASS:-}" && ! -z "${SIGNCERTID:-}" ]]
#then
#  signfile "${OUTPUTDIR}/${APPNAME}-${SHORTVERSION}.dmg"
#fi

# Create intermediate component package with pkgbuild, using DoxComponent.plist
# that was previously generated via
# "pkgbuild --analyze --root . DoxComponent.plist" from the directory the .app is in

PKGROOT="${BASEDIR}/build/${BUILDTYPE}/${SCRATCH}"

cd "${PKGROOT}" || die "bad pkgroot"
cd .. || die "something bad"

pkgbuild --root "${PKGROOT}" --component-plist "${BASEDIR}/package/DoxComponent.plist" "${APPNAME}Component.pkg" || die "Failed to create component package" || die "pkgbuild failed"

# Build ourselves a distribution plist

productbuild --synthesize --product "${BASEDIR}/package/requirements.plist" --package "${APPNAME}Component.pkg" distribution.plist || die "Failed to synthesize distribution.plist"

# TODO(steve): Edit distribution.plist to add README, LICENSE, ...

productbuild --distribution distribution.plist --resources . "${APPNAME}.pkg" || die "Failed to create ${APPNAME}.pkg"

if [[ ! -z "${INSTALLERCERTID:-}" ]]
then
  productsign --sign="${INSTALLERCERTID}" --keychain="${SIGNKCFILE}" "${APPNAME}.pkg" "${OUTPUTDIR}/${APPNAME}.pkg" || die "Failed to sign package"
else
  cp "${APPNAME}.pkg" "${OUTPUTDIR}/${APPNAME}.pkg" || die "Failed to copy package package"
fi

rm -rf dist
mkdir -p dist || die "failed to create dist directory for packaging"
cp "${OUTPUTDIR}/${APPNAME}.pkg" "dist/${APPNAME}.pkg" || die "failed to copy installer to dmg scratch dir"
rm -f "${OUTPUTDIR}/${APPNAME}-${SHORTVERSION}.dmg"
hdiutil create -volname "${APPNAME} ${SHORTVERSION}" -srcfolder ./dist -ov "${OUTPUTDIR}/${APPNAME}-${SHORTVERSION}.dmg" || die "failed to create final dmg"

#productbuild --product "${APPNAME}.app" "/Applications/${APPNAME}.app" "${OUTPUTDIR}/${APPNAME}-${SHORTVERSION}.pkg"

if [[ "${BUILDKITE:-false}" == "true" ]]
then
# upload artifact
cd ${OUTPUTDIR}
cd ..
${BKAGENT} artifact upload "**/${APPNAME}-${SHORTVERSION}.dmg" || die "failed to upload artifacts" 
fi

