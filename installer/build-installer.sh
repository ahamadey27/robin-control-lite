#!/usr/bin/env bash
# build-installer.sh — Build the macOS .pkg installer for Robin Control Lite.
#
# Inputs (must exist before running):
#   NewProject/build-release/RobinControlLite_artefacts/Release/
#     ├── VST3/Robin Control Lite.vst3
#     └── AU/Robin Control Lite.component
#
# Output:
#   Releases/Installers/Robin Control Lite <VERSION>.pkg
#
# Signing (deferred — runs unsigned by default):
#   When the Apple Developer ID Installer cert is in your keychain, set
#     INSTALLER_SIGN="Developer ID Installer: CONDUIT DSP LLC (TEAMID)"
#   in the environment and re-run. The flag drops into productbuild as
#   --sign "$INSTALLER_SIGN".

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
ARTEFACTS="${RCL_ARTEFACTS:-$REPO_ROOT/NewProject/build-release/RobinControlLite_artefacts/Release}"
OUTPUT_DIR="$REPO_ROOT/Releases/Installers"
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
for fmt_dir in "VST3/Robin Control Lite.vst3" \
               "AU/Robin Control Lite.component"; do
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
done

mkdir -p "$OUTPUT_DIR" "$STAGE_DIR/pkgs" "$STAGE_DIR/resources"

# Optional --sign flag (set INSTALLER_SIGN env var to enable)
SIGN_FLAGS=()
if [[ -n "${INSTALLER_SIGN:-}" ]]; then
    SIGN_FLAGS=(--sign "$INSTALLER_SIGN")
    echo "==> Signing with: $INSTALLER_SIGN"
else
    echo "==> Building UNSIGNED (set INSTALLER_SIGN env var to sign)"
fi

# 1. Per-format component pkgs
echo "==> pkgbuild VST3"
pkgbuild \
    --component "$ARTEFACTS/VST3/Robin Control Lite.vst3" \
    --identifier "dsp.conduit.RobinControlLite.vst3" \
    --version "$VERSION" \
    --install-location "/Library/Audio/Plug-Ins/VST3" \
    "$STAGE_DIR/pkgs/RobinControlLite-VST3.pkg"

echo "==> pkgbuild AU"
pkgbuild \
    --component "$ARTEFACTS/AU/Robin Control Lite.component" \
    --identifier "dsp.conduit.RobinControlLite.au" \
    --version "$VERSION" \
    --install-location "/Library/Audio/Plug-Ins/Components" \
    "$STAGE_DIR/pkgs/RobinControlLite-AU.pkg"

# 2. Installer GUI resources (license shown during install)
cp "$REPO_ROOT/EULA.md" "$STAGE_DIR/resources/license.txt"

# 3. Substitute @VERSION@ placeholder in distribution.xml
sed "s/@VERSION@/${VERSION}/g" \
    "$SCRIPT_DIR/distribution.xml" \
    > "$STAGE_DIR/distribution.xml"

# 4. Build the distribution pkg
OUTPUT_PKG="$OUTPUT_DIR/Robin Control Lite ${VERSION}.pkg"
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
