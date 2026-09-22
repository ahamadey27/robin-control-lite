#!/usr/bin/env bash
# uninstall.sh — Remove Robin Control Lite plugin bundles and installer receipts.
#
# Removes plugin bundles from /Library (system-wide install paths) and
# user-local dev copies in ~/Library, then clears pkgutil's receipt database
# for a clean future re-install. Presets, samples and licensing data are preserved.

set -e

PLUGIN_NAME="Robin Control Lite"

confirm() {
    printf "%s [y/N] " "$1"
    read -r response
    [[ "$response" =~ ^[yY]$ ]]
}

echo "This will remove:"
echo "  /Library/Audio/Plug-Ins/VST3/${PLUGIN_NAME}.vst3"
echo "  /Library/Audio/Plug-Ins/Components/${PLUGIN_NAME}.component"
echo "  ~/Library/Audio/Plug-Ins/VST3/${PLUGIN_NAME}.vst3 (if present)"
echo "  ~/Library/Audio/Plug-Ins/Components/${PLUGIN_NAME}.component (if present)"
echo "  /Library/Application Support/Avid/Audio/Plug-Ins/${PLUGIN_NAME}.aaxplugin"
echo "  ~/Library/Application Support/Avid/Audio/Plug-Ins/${PLUGIN_NAME}.aaxplugin (if present)"
echo "  pkgutil receipts: dsp.conduit.RobinControlLite.{vst3,au,aax}"
echo ""

if ! confirm "Continue?"; then
    echo "Aborted."
    exit 0
fi

echo "Requesting sudo for system-wide removals..."
sudo rm -rf "/Library/Audio/Plug-Ins/VST3/${PLUGIN_NAME}.vst3"
sudo rm -rf "/Library/Audio/Plug-Ins/Components/${PLUGIN_NAME}.component"
sudo rm -rf "/Library/Application Support/Avid/Audio/Plug-Ins/${PLUGIN_NAME}.aaxplugin"

# User-local copies (placed by COPY_PLUGIN_AFTER_BUILD during dev)
rm -rf "$HOME/Library/Audio/Plug-Ins/VST3/${PLUGIN_NAME}.vst3"
rm -rf "$HOME/Library/Audio/Plug-Ins/Components/${PLUGIN_NAME}.component"
rm -rf "$HOME/Library/Application Support/Avid/Audio/Plug-Ins/${PLUGIN_NAME}.aaxplugin"

# Clear pkgutil receipts so a fresh install is clean
sudo pkgutil --forget dsp.conduit.RobinControlLite.vst3 2>/dev/null || true
sudo pkgutil --forget dsp.conduit.RobinControlLite.au   2>/dev/null || true
sudo pkgutil --forget dsp.conduit.RobinControlLite.aax  2>/dev/null || true

echo ""
echo "Robin Control Lite has been uninstalled."
echo "If a DAW had this plugin loaded in a project, restart the DAW for the change to take effect."
