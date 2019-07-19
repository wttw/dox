# -*-bash-*-

# Make sure we're in the right directory and the app was built
[[ -f "${APPNAME}.app/Contents/MacOS/${APPNAME}" ]] || die "${APPNAME} wasn't built"

# TODO(steve): codesigning

macdeployqt "${APPNAME}.app" -dmg || die "macdeployqt failed"
mv "${APPNAME}.dmg" "${OUTPUTDIR}/${APPNAME}-${SHORTVERSION}.dmg" || die "Failed to move DMG to ${OUTPUTDIR}"

cd ${OUTPUTDIR}
cd ..
${BKAGENT} artifact upload "**/${APPNAME}-${SHORTVERSION}.dmg" || die "failed to upload artifacts" 
