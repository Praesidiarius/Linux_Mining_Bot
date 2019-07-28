#!/bin/bash
#================================================================
# HEADER
#================================================================
#% SYNOPSIS
#+    ${SCRIPT_NAME}
#%
#% DESCRIPTION
#%    This script is checking for running miners
#%    if no miners are found, start script will be
#%    executed. this script also reports to dashboard
#%    server.
#%
#================================================================
#- IMPLEMENTATION
#-    version         ${SCRIPT_NAME} (www.free-miners.org) 0.7.1
#-    author          Jürg Binggeli, Nathanael Kammermann
#-    copyright       Copyright (c) http://www.free-miners.org
#-    license         GNU General Public License
#-    script_id       scm-check-1
#-
#================================================================
#  HISTORY
#     2018/05/02 : nkammermann : Dashboard URL + Licence Change
#     2018/04/05 : nkammermann : Script overhaul
#     2018/03/01 : jbinggeli : Watchdog added, improvements
#     2017/12/22 : nkammermann : Lots of improvements
#     2017/07/10 : nkammermann : Script creation
#
#================================================================
#  DEBUG OPTION
#    not yet ..
#
#================================================================
# END_OF_HEADER
#================================================================

#== general variables for script ==#
DASHBOARDHOST="https://dashboard.free-miners.org"
DASHBOARDUPDATEURL="${DASHBOARDHOST}/rigadmin/updatestate"
DASHBOARDSETTINGSURL="${DASHBOARDHOST}/rigadmin/getsettings"
SUBMODE=0
GPUCOUNT=0

#== network card information ==#
NETCARD="eth0"
for i in /sys/class/net/* ; do
  if [ -d "$i" ]; then
   if [ "$i" != "/sys/class/net/lo" ]; then
     NETCARD="$i"
   fi
  fi
done
HWADDR=`cat ${NETCARD}/address`
IFS='/' read -r -a array <<< "$NETCARD"
NETDEV="${array[4]}"
echo "CARD: $NETCARD"
echo "DEV: $NETDEV"
IPADDR=`/sbin/ifconfig $NETDEV | awk '/inet /{print substr($2, 1)}'`
MINERINFO="Hashrate Payment done. Restarting."
NAMESTRING=""
RIGINFO=`/home/dbadmin/curl-7.58.0/src/curl -X POST -s -d "rig-hw-addr=${HWADDR}" ${DASHBOARDHOST}/rigadmin/hashdone`
echo $RIGINFO
sudo crontab /home/dbadmin/scm/origcron.settings
rm -f /home/dbadmin/scm/origcron.settings
rm -f /home/dbadmin/scm/tempcron.settings
