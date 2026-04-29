#!/bin/sh
set -e
if test "$CONFIGURATION" = "Custom"; then :
  cd /Users/alex/Documents/Github/robin-control-redesign/NewProject/build-release/JUCE/tools
  make -f /Users/alex/Documents/Github/robin-control-redesign/NewProject/build-release/JUCE/tools/CMakeScripts/ReRunCMake.make
fi

