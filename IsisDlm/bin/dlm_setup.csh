#!/bin/csh
#########################################################################
#_Title dlm_setup.csh Sets up ISIS 3 DLM for use
#
#       Users of this file must set the ISISDLM environment variable
#       to the top level of the ISIS Dlm installation directory.
#
#       Before sourcing this file, user must set the environment
#       variable ISISDLM.  Follow is an example of how this is
#       acheived:
#
#          setenv ISISDLM /usgs/pkgs/isis/dlm
#
#_Hist  Mar 08 2005 Kris Becker, USGS, Flagstaff Original Version
#_End
#########################################################################

#  Ensure the user has setup the root path
if (${?ISISDLM} == 0) then
  echo "Please set ISISDLM to the installation directory"
  exit 1
endif

set DlmDir = "$ISISDLM/lib"
if (${?IDL_DLM_PATH} == 0) then
  setenv IDL_DLM_PATH "<IDL_DEFAULT>:+"$DlmDir
else 
#  Check to determine if its already in the path
  echo $IDL_DLM_PATH | grep $DlmDir >& /dev/null
  if ($status != 0) then
    setenv IDL_DLM_PATH ${IDL_DLM_PATH}:+$DlmDir
  endif
endif

#  Clean up and exit
unset DlmDir
exit 0
