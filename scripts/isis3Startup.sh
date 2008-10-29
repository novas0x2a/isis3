#!/bin/sh
#
# This file should be executed within your current shell using the "." command
# Since this is only a beta version we do not suggest you add this
# command to your startup file
#
# On the command line type:
# > set ISISROOT=????
# > . isis3Startup.sh
#
# Replace the "????" in the above command line with the path you installed
# the Isis distribution
#
if [ ! "$ISISROOT" ]; then
  ISISROOT=/usgs/pkgs/isis3/isis
  export ISISROOT
fi

if [ -d $ISISROOT/../data ]; then
  ISIS3DATA=$ISISROOT/../data
else
  ISIS3DATA=/usgs/cpkgs/isis3/data
fi
export ISIS3DATA

if [ -d $ISISROOT/../testData ]; then
  ISIS3TESTDATA=$ISISROOT/../testData
else
  ISIS3TESTDATA=/usgs/cpkgs/isis3/testData
fi
export ISIS3TESTDATA

if [ "$PATH" ]; then
  PATH="${PATH}:${ISISROOT}/bin"
else
  PATH="$ISISROOT/bin"
fi
export PATH

# Create QT_PLUGIN_PATH env variable
QT_PLUGIN_PATH="$ISISROOT/3rdParty/plugins"
export QT_PLUGIN_PATH

Platform=`uname -s`

# Initialize Mac OS X
if [ $Platform = "Darwin" ]; then
  if [ "$DYLD_FALLBACK_LIBRARY_PATH" ]; then
     DYLD_FALLBACK_LIBRARY_PATH="${DYLD_FALLBACK_LIBRARY_PATH}:${ISISROOT}/lib:${ISISROOT}/3rdParty/lib"
 else
     DYLD_FALLBACK_LIBRARY_PATH="${ISISROOT}/lib:${ISISROOT}/3rdParty/lib"
 fi

  export DYLD_FALLBACK_LIBRARY_PATH

else 
# Set up other Unixes
  if [ "$LD_LIBRARY_PATH" ]; then
    LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${ISISROOT}/lib"
  else
    LD_LIBRARY_PATH=${ISISROOT}/lib:
  fi

  LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${ISISROOT}/3rdParty/lib"

  if [ "`uname`" = "SunOS" ]; then
    LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:/usr/lib/sparcv9"
  fi

  export LD_LIBRARY_PATH
fi
