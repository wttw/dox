# -*-bash-*-

# We're called with BUILDTYPE, BASEDIR, OUTPUTDIR, APPNAME, VERSION, SHORTVERSION
# and cwd $BASEDIR/build/$BUILDTYPE
# These are *wsl* paths, e.g. /mnt/c/whatever not
# windows paths, so need to use wslpath -m to convert
# them for use by windows .exes
# SIGNINGCERT is set in environment.bat, and is a windows path

function signfile {
  file="$1"
  description="$2"

  if [[ -z "${SIGNINGCERT:-}" || -z "${SIGNINGPASS:-}" ]]
  then
    echo "Certificate not provided, not signing $file" 1>&2
    return 0
  fi

  if [[ ! -f $(wslpath -u "$SIGNINGCERT") ]]
  then
    die "$SIGNINGCERT isn't there for $file"
  fi
  TIMESTAMPSERVERS="http://timestamp.comodoca.com/authenticode http://timestamp.verisign.com/scripts/timestamp.dll http://timestamp.globalsign.com/scripts/timestamp.dll http://tsa.starfieldtech.com"
  for attempt in {1..120}
  do
    for server in $TIMESTAMPSERVERS; do
      if signtool.exe sign /v /f "$SIGNINGCERT" /p "$SIGNINGPASS" /tr $server /d "$description" $file; then
        echo "Signed $file with $server" 1>&2
        return 0
      fi
      echo "Failed to sign $file with $server" 1>&2
    done
  sleep 1
  done
  die "Failed to sign $file after many attempts"
}


WSRCDIR=$(wslpath -m "${BASEDIR}")
WBINDIR=$(wslpath -m "${BASEDIR}/build/${BUILDTYPE}/scratch/${APPNAME}")

echo WSRCDIR = "${WSRCDIR}"
echo WBINDIR = "${WBINDIR}"

# Make sure we're in the right directory and the app was built
[[ -f "${APPNAME}.exe" ]] || die "${APPNAME} wasn't built"

rm -rf "scratch/${APPNAME}"
mkdir -p "scratch/${APPNAME}" || die "Failed to create scratch directory"
cp "${APPNAME}.exe" "scratch/${APPNAME}/${APPNAME}.exe" || die "Failed to copy ${APPNAME}.exe"
windeployqt.exe --verbose 0 "scratch/${APPNAME}/${APPNAME}.exe" || die "windeployqt failed"

# Sign the executable
signfile "scratch/${APPNAME}/${APPNAME}.exe" "${DESCRIPTION}"

# Create a zip package
cd scratch
#zip -9r "${OUTPUTDIR}/${APPNAME}.zip" "${APPNAME}"
cd ..

# Create an installer

# Build wix config fragment for all files except the exe
cd scratch
heat.exe dir "${APPNAME}" -t ../../../package/remove-exe.xslt -cg QtDlls -ag -scom -sreg -sfrag -srd -dr APPLICATIONFOLDER -var var.bindir -template fragment -out ../dir.wxs  || die "heat failed"
cd ..

# Compile our wix config fragments
candle.exe "-dVersion=${SHORTVERSION}" "-dLongVersion=${VERSION}" "-dbindir=${WBINDIR}" "-dsrcdir=${WSRCDIR}" ../../package/dox.wxs || die "failed to candle main exe"
candle.exe "-dVersion=${SHORTVERSION}" "-dLongVersion=${VERSION}" "-dbindir=${WBINDIR}" "-dsrcdir=${WSRCDIR}" dir.wxs || die "failed to candle harvested files"

MSI="dox-${SHORTVERSION}.msi"

# And build the msi
light.exe -ext WixUIExtension -out "${MSI}" dox.wixobj dir.wixobj || die "light failed"

# sign the msi
signfile "${MSI}" "${DESCRIPTION} installer"

mv "${MSI}" "${OUTPUTDIR}/${MSI}" || die "Failed to move MSI to ${OUTPUTDIR}"

echo "Artefact at ${OUTPUTDIR}/${MSI}" 1>&2

