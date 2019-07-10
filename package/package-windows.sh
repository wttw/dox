# -*-bash-*-

# Make sure we're in the right directory and the app was built
[[ -f "${APPNAME}.exe" ]] || die "${APPNAME} wasn't built"

# TODO(steve): codesigning, wix

rm -rf "scratch/${APPNAME}"
mkdir -p "scratch/${APPNAME}"
mv "${APPNAME}.exe" "scratch/${APPNAME}/${APPNAME}.exe"
windeployqt.exe "scratch/${APPNAME}/${APPNAME}.exe"
cd scratch
zip -9r "${OUTPUTDIR}/${APPNAME}.zip" "${APPNAME}"
