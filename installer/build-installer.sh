#!/usr/bin/env bash
# build-installer.sh — Build the macOS .pkg installer for Robin Control Lite.
#
# Inputs (must exist before running):
#   NewProject/build-release/RobinControlLite_artefacts/Release/
#     ├── VST3/Robin Control Lite.vst3
#     └── AU/Robin Control Lite.component
#     └── AAX/Robin Control Lite.aaxplugin (PACE-signed; RCL_INCLUDE_AAX=1)
#
# RCL_ARTEFACTS selects signed staging. RCL_BETA_LABEL and RCL_OUTPUT_DIR
# select a private test package name/location; beta packages show the included
# activation instructions. Existing output files are never overwritten.
#
# Output:
#   Releases/Installers/Robin Control Lite <VERSION>.pkg
#
# Signing (required for final; optional for explicitly labelled beta packages):
#   When the Apple Developer ID Installer cert is in your keychain, set
#     INSTALLER_SIGN="Developer ID Installer: CONDUIT DSP LLC (TEAMID)"
#   in the environment and re-run. The flag drops into productbuild as
#   --sign "$INSTALLER_SIGN".

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
ARTEFACTS="${RCL_ARTEFACTS:-$REPO_ROOT/NewProject/build-release/RobinControlLite_artefacts/Release}"
OUTPUT_DIR="${RCL_OUTPUT_DIR:-$REPO_ROOT/Releases/Installers}"
INCLUDE_AAX="${RCL_INCLUDE_AAX:-0}"
BETA_LABEL="${RCL_BETA_LABEL:-}"
if [[ "$INCLUDE_AAX" != 0 && "$INCLUDE_AAX" != 1 ]]; then
    echo "ERROR: RCL_INCLUDE_AAX must be 0 or 1" >&2
    exit 1
fi
if [[ "$BETA_LABEL" == *'/'* || "$BETA_LABEL" == *$'\n'* ]]; then
    echo "ERROR: beta label must be a single filename-safe line" >&2
    exit 1
fi
STAGE_DIR="$(mktemp -d -t rcl-installer)"
trap 'rm -rf "$STAGE_DIR"' EXIT

# Single source of truth for version: NewProject/CMakeLists.txt
CMAKE_FILE="$REPO_ROOT/NewProject/CMakeLists.txt"
VERSION="$(grep -E '^project\(RobinControlLite VERSION' "$CMAKE_FILE" \
            | sed -E 's/.*VERSION ([0-9]+\.[0-9]+\.[0-9]+).*/\1/')"

if [[ -z "${VERSION:-}" ]]; then
    echo "ERROR: could not parse VERSION from $CMAKE_FILE" >&2
    exit 1
fi

echo "==> Robin Control Lite installer build"
echo "    version:   $VERSION"
echo "    artefacts: $ARTEFACTS"
echo "    staging:   $STAGE_DIR"
echo "    output:    $OUTPUT_DIR"

# Verify the Release artefacts exist
FORMATS=("VST3/Robin Control Lite.vst3" "AU/Robin Control Lite.component")
if [[ "$INCLUDE_AAX" == 1 ]]; then
    FORMATS+=("AAX/Robin Control Lite.aaxplugin")
fi
for fmt_dir in "${FORMATS[@]}"; do
    if [[ ! -d "$ARTEFACTS/$fmt_dir" ]]; then
        echo "ERROR: missing artefact $ARTEFACTS/$fmt_dir" >&2
        echo "       Run a Release build first:" >&2
        echo "         cd NewProject && cmake --build build-release --config Release" >&2
        exit 1
    fi
    bundle_version="$(/usr/libexec/PlistBuddy -c 'Print :CFBundleShortVersionString' \
        "$ARTEFACTS/$fmt_dir/Contents/Info.plist")"
    if [[ "$bundle_version" != "$VERSION" ]]; then
        echo "ERROR: $fmt_dir is version $bundle_version; expected $VERSION. Refusing to package stale artifacts." >&2
        exit 1
    fi
    if [[ -z "$BETA_LABEL" ]]; then
        profile="$ARTEFACTS/$fmt_dir/Contents/Resources/RCLBuildConfig.txt"
        if [[ ! -f "$profile" ]] || ! grep -Fxq 'moonbase=ON' "$profile" \
            || ! grep -Fxq 'final=ON' "$profile" || ! grep -Fxq "version=$VERSION" "$profile"; then
            echo "ERROR: final installer requires final-release Moonbase artifacts: $fmt_dir" >&2
            exit 1
        fi
        test -f "$ARTEFACTS/$fmt_dir/Contents/Resources/MoonbaseNotices.txt"
        codesign --verify --strict "$ARTEFACTS/$fmt_dir"
    fi
done

if [[ -z "$BETA_LABEL" && -z "${INSTALLER_SIGN:-}" ]]; then
    echo "ERROR: final installer requires INSTALLER_SIGN" >&2
    exit 1
fi

if [[ "$INCLUDE_AAX" == 1 ]]; then
    # AAX for retail Pro Tools must be PACE-signed, not a raw JUCE build.
    test -f "$ARTEFACTS/AAX/Robin Control Lite.aaxplugin/Contents/Resources/RobinControlLitePages.xml"
    codesign --verify --strict "$ARTEFACTS/AAX/Robin Control Lite.aaxplugin"
    /Applications/PACEAntiPiracy/Eden/Fusion/Current/bin/wraptool verify \
        --in "$ARTEFACTS/AAX/Robin Control Lite.aaxplugin"
fi

mkdir -p "$OUTPUT_DIR" "$STAGE_DIR/pkgs" "$STAGE_DIR/resources"

# Optional --sign flag (set INSTALLER_SIGN env var to enable)
SIGN_FLAGS=()
if [[ -n "${INSTALLER_SIGN:-}" ]]; then
    SIGN_FLAGS=(--sign "$INSTALLER_SIGN")
    echo "==> Signing with: $INSTALLER_SIGN"
else
    echo "==> Building UNSIGNED (set INSTALLER_SIGN env var to sign)"
fi

# 1. Fixed-destination payloads. Never relocate an install to a development or
# backup bundle discovered elsewhere on the disk.
build_component() {
    local format="$1" extension="$2" destination="$3" suffix="$4"
    local payload="$STAGE_DIR/payload-$format"
    local components="$STAGE_DIR/components-$format.plist"
    mkdir -p "$payload"
    ditto "$ARTEFACTS/$format/Robin Control Lite.$extension" "$payload/Robin Control Lite.$extension"
    pkgbuild --analyze --root "$payload" "$components"
    python3 - "$components" <<'PY'
import plistlib, sys
from pathlib import Path
path = Path(sys.argv[1])
entries = plistlib.loads(path.read_bytes())
for entry in entries:
    entry['BundleIsRelocatable'] = False
    entry['BundleHasStrictIdentifier'] = True
    entry['BundleOverwriteAction'] = 'upgrade'
path.write_bytes(plistlib.dumps(entries))
PY
    pkgbuild --root "$payload" --component-plist "$components" \
        --identifier "dsp.conduit.RobinControlLite.$suffix" --version "$VERSION" \
        --install-location "$destination" "$STAGE_DIR/pkgs/RobinControlLite-$format.pkg"
}
build_component VST3 vst3 /Library/Audio/Plug-Ins/VST3 vst3
build_component AU component /Library/Audio/Plug-Ins/Components au
if [[ "$INCLUDE_AAX" == 1 ]]; then
    build_component AAX aaxplugin "/Library/Application Support/Avid/Audio/Plug-Ins" aax
fi

# 2. Installer GUI resources (license shown during install)
cp "$REPO_ROOT/EULA.md" "$STAGE_DIR/resources/license.txt"

# 3. Substitute @VERSION@ placeholder in distribution.xml
python3 - "$SCRIPT_DIR/distribution.xml" "$STAGE_DIR/distribution.xml" "$VERSION" "$INCLUDE_AAX" "$BETA_LABEL" <<'PY'
import sys
import xml.etree.ElementTree as ET
source, destination, version, include_aax, beta = sys.argv[1:]
tree = ET.parse(source)
root = tree.getroot()
if include_aax != '1':
    outline = root.find('choices-outline')
    for node in list(outline):
        if node.get('choice') == 'aax': outline.remove(node)
    for node in list(root):
        if node.get('id') in ('aax', 'dsp.conduit.RobinControlLite.aax'): root.remove(node)
if beta:
    root.find('title').text = 'Robin Control Lite ' + beta
    # This private test installer presents activation instructions. Published
    # release terms still need the review recorded in MOONBASE_INTEGRATION.md.
    root.remove(root.find('license'))
    ET.SubElement(root, 'welcome', {'file': 'beta-welcome.txt'})
for node in root.findall('pkg-ref'):
    if node.get('version') == '@VERSION@': node.set('version', version)
tree.write(destination, encoding='utf-8', xml_declaration=True)
PY
if [[ -n "$BETA_LABEL" ]]; then
    cp "$SCRIPT_DIR/beta-welcome.txt" "$STAGE_DIR/resources/beta-welcome.txt"
fi

# 4. Build the distribution pkg
OUTPUT_PKG="$OUTPUT_DIR/Robin Control Lite ${VERSION}.pkg"
if [[ -n "$BETA_LABEL" ]]; then
    OUTPUT_PKG="$OUTPUT_DIR/Robin Control Lite ${VERSION} ${BETA_LABEL}.pkg"
fi
if [[ -e "$OUTPUT_PKG" ]]; then
    echo "ERROR: output already exists; choose a new beta label or output directory: $OUTPUT_PKG" >&2
    exit 1
fi
echo "==> productbuild → $OUTPUT_PKG"
productbuild \
    --distribution "$STAGE_DIR/distribution.xml" \
    --package-path "$STAGE_DIR/pkgs" \
    --resources "$STAGE_DIR/resources" \
    ${SIGN_FLAGS[@]+"${SIGN_FLAGS[@]}"} \
    "$OUTPUT_PKG"

echo ""
echo "==> Done."
ls -lh "$OUTPUT_PKG"

if [[ -z "${INSTALLER_SIGN:-}" ]]; then
    cat <<EOF

The pkg is UNSIGNED. macOS Gatekeeper will block double-click installation
on a Mac that has never had this plugin installed. To install for testing:
    sudo installer -pkg "$OUTPUT_PKG" -target /
EOF
fi
