#!/bin/sh
#########################################################################
#_Title dlm_setup.sh Sets up ISIS 3 DLM for use in Bash shell
#
#       Users of this file must set the ISISDLM environment variable
#       to the top level of the ISIS Dlm installation directory.
#
#       Before sourcing this file, user must set the environment
#       variable ISISDLM.  Follow is an example of how this is
#       acheived:
#
#          ISISDLM=/usgs/pkgs/isis/dlm
#          export ISISDLM
#
#_Hist  SEP 15 2005 Jac Shinaman, USGS, Flagstaff Original Version
#
#_Ver $Id$
#
#_End
#########################################################################

#  Ensure the user has setup the root path
if [ ! "$ISISDLM" ]; then
  echo "Please set ISISDLM to the installation directory"
  exit 1
fi

DlmDir=$ISISDLM/lib
if [ ! "$IDL_DLM_PATH" ]; then
  IDL_DLM_PATH="<IDL_DEFAULT>:+"$DlmDir
  export IDL_DLM_PATH
else 
#  Check to determine if its already in the path
  echo $IDL_DLM_PATH | grep $DlmDir > /dev/null 2>&1
  if [ $? -ne 0 ]; then
    IDL_DLM_PATH=${IDL_DLM_PATH}:+$DlmDir
    export IDL_DLM_PATH 
  fi
fi

#  Clean up and exit
unset DlmDir
