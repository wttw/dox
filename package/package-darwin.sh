# -*-bash-*-

# Make sure we're in the right directory and the app was built
[[ -f "${APPNAME}.app/Contents/MacOS/${APPNAME}" ]] || die "${APPNAME} wasn't built"

macdeployqt "${APPNAME}.app" -dmg
mv "${APPNAME}.dmg" "${OUTPUTDIR}/${APPNAME}.dmg"
