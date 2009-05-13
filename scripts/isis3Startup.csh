#!/bin/csh
################################################################################
# This file should be sourced within your current shell using the "source" 
# command.  Since this is only a beta version we do not suggest you add this 
# command to your startup file
#
# On the command line type:
# > setenv ISISROOT ????
# > source isis3Startup.csh
#
# Replace the "????" in the above command line with the path you installed
# the Isis distribution
#
#_HIST  FEB 21 2006 - Jac Shinaman - USGS, Astrogeology - added code to prevent
#               duplication of paths
#       FEB 22 2006 - JRS - changed OsType to Platform to avoid conflicts with
#               internal USGS initIsis.csh script
#       JUL 12 2006 - Robert Wallace - Add ISIS3TESTDATA environment variable
#       AUG 25 2008 - Kris Becker - Changed all references of DYLD_LIBRARY_PATH
#                       to DYLD_FALLBACK_LIBRARY_PATH.  See 
#                       http://www.osxfaq.com/man/1/dyld.ws.
#       MAR 12 2009 - Christopher Austin - Changed the way ISIS3TESTDATE is set
#                       to prevent its setting for outside groups and default
#                       to "/usgs/cpkgs/isis3/testData"
#_VER   $Id: isis3Startup.csh,v 1.4 2009/03/13 00:18:35 caustin Exp $
#_END
################################################################################
# Check parameters
set QTPLUGINPATH = "true"
foreach arg ($argv)
  if ( "$arg" == "-noqtpluginpath" ) then
    set QTPLUGINPATH = "false"
  else
    echo "Uknown argument $arg"
    exit 1
  endif
end

# Establish a platform switch variable
set Platform = `uname -s`

# Initialize the ISISROOT environment variable if it doesn't exist
# TODO: Test is obsolete, fail if no isisroot is set
if ($?ISISROOT == 0) then
  setenv ISISROOT /usgs/pkgs/isis3/isis
endif

# Initialize the ISIS3DATA environment variable
if (-d $ISISROOT/../data) then
  setenv ISIS3DATA $ISISROOT/../data
else
  setenv ISIS3DATA /usgs/cpkgs/isis3/data
endif

# Initialize the ISIS3TESTDATA environment variable
if (-d /usgs/cpkgs/isis3/testData) then
  setenv ISIS3TESTDATA /usgs/cpkgs/isis3/testData
else
  if (-d $ISISROOT/../testData) then
    setenv ISIS3TESTDATA $ISISROOT/../testData
  endif
endif

# Insert ISISROOT/bin in the PATH environment variable if it's not already there
if ($?PATH == 0) then
  setenv PATH "$ISISROOT/bin"
else
  printenv PATH |grep $ISISROOT/bin >& /dev/null
  if  ( $status != 0) then
    setenv PATH "${PATH}:$ISISROOT/bin"
  endif
endif

# Create QT_PLUGIN_PATH env variable
if ($QTPLUGINPATH == "true") then
  setenv QT_PLUGIN_PATH "$ISISROOT/3rdParty/plugins"
endif

# Initialize the platform specific library paths if they don't already exist
if ($Platform == "Darwin") then
  if ($?DYLD_FALLBACK_LIBRARY_PATH == 0) then
    setenv DYLD_FALLBACK_LIBRARY_PATH ${ISISROOT}/lib
  else
    printenv DYLD_FALLBACK_LIBRARY_PATH | grep ${ISISROOT}/lib >& /dev/null
    if  ( $status != 0) then
      setenv DYLD_FALLBACK_LIBRARY_PATH "${DYLD_FALLBACK_LIBRARY_PATH}:${ISISROOT}/lib"
    endif
  endif
  printenv DYLD_FALLBACK_LIBRARY_PATH | grep ${ISISROOT}/3rdParty/lib >& /dev/null
  if  ( $status != 0) then  
    setenv DYLD_FALLBACK_LIBRARY_PATH "${DYLD_FALLBACK_LIBRARY_PATH}:${ISISROOT}/3rdParty/lib"
  endif
else
  if ($?LD_LIBRARY_PATH == 0) then
    setenv LD_LIBRARY_PATH ${ISISROOT}/lib
  else
    printenv LD_LIBRARY_PATH | grep ${ISISROOT}/lib >& /dev/null
    if  ( $status != 0) then
      setenv LD_LIBRARY_PATH "${LD_LIBRARY_PATH}:${ISISROOT}/lib"
    endif  
  endif
  printenv LD_LIBRARY_PATH | grep ${ISISROOT}/3rdParty/lib >& /dev/null
  if  ( $status != 0) then
    setenv LD_LIBRARY_PATH "${LD_LIBRARY_PATH}:${ISISROOT}/3rdParty/lib"
  endif
  if ($Platform == "SunOS") then
    printenv LD_LIBRARY_PATH | grep /usr/lib/sparcv9 >& /dev/null
    if  ( $status != 0) then
      setenv LD_LIBRARY_PATH "${LD_LIBRARY_PATH}:/usr/lib/sparcv9"
    endif
  endif
endif

unset Platform
