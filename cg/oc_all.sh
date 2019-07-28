#!/bin/bash

# oc_all.sh  This script will set OC for core, memory and power limit for all GPU's
#
# Description:  A script to control GPU core, memory and power limit on headless (non-X) linux nodes

# Original Script by Axel Kohlmeyer <akohlmey@gmail.com>
# https://sites.google.com/site/akohlmey/random-hacks/nvidia-gpu-coolness
#
# Modified for newer drivers and removed old work-arounds
# Tested on Ubuntu 14.04 with driver 352.41
# Copyright 2015, squadbox

# Requirements:
# * An Nvidia GPU
# * Nvidia Driver V285 or later
# * xorg
# * Coolbits enabled and empty config setting
#     nvidia-xconfig -a --cool-bits=28 --allow-empty-initial-configuration

# You may have to run this as root or with sudo if the current user is not authorized to start X sessions.

# Paths to the utilities we will need
SMI='/usr/bin/nvidia-smi'
SET='/usr/bin/nvidia-settings'

PLMAX=250
OCCOREMAX=250
OCCOREMIN=-200
OCMEMMAX=2000
OCMEMMIN=-1000

# Determine major driver version
VER=`awk '/NVIDIA/ {print $8}' /proc/driver/nvidia/version | cut -d . -f 1`

# Drivers from 285.x.y on allow persistence mode setting
if [ ${VER} -lt 285 ]
then
    echo "Error: Current driver version is ${VER}. Driver version must be greater than 285."; exit 1;
fi

declare -a GPUNAMELIST
IFS=$'\n'       # make newlines the only separator
y=0
for j in $(nvidia-smi --query-gpu=gpu_name --format=csv,noheader)
do
        y=$((y+1))
        GPUNAMELIST[$y]=$j
done
# how many GPU's are in the system?
NUMGPU="$(nvidia-smi -L | wc -l)"
MAXGPU=$((NUMGPU-1))

# Read numerical command line args 
if [ "$1" -eq "$1" ] 2>/dev/null && [[ "$1" -ge "$OCCOREMIN" ]]  && [ "$1" -le "$OCCOREMAX" ] && 
   [[ "$2" -ge "$OCMEMMIN" ]]  && [ "$2" -le "$OCMEMMAX" ] &&   
   [ "$3" -eq "$3" ] 2>/dev/null &&
   [ "0$3" -ge "0$PLMIN" ]  &&
   [ "$3" -le "$PLMAX" ] 
           
then
    core=$1  
    mem=$2
    pl=$3
    $SMI -pm 1 1>/dev/null # enable persistance mode
    tmp=$(mktemp)
    SCM='/home/rigadmin/scm'

    # loop through each GPU and individually set fan speed
    n=0
    while [  $n -lt  $NUMGPU ];
    do
        DISPLAY=:0 XAUTHORITY=/var/run/lightdm/root/:0 ${SET} -a [gpu:${n}]/GPUMemoryTransferRateOffset[3]=$mem 1>/dev/null
        DISPLAY=:0 XAUTHORITY=/var/run/lightdm/root/:0 ${SET} -a [gpu:${n}]/GPUGraphicsClockOffset[3]=$core 1>/dev/null
        DISPLAY=:0 XAUTHORITY=/var/run/lightdm/root/:0 ${SMI} -i ${n} -pl $pl 1>/dev/null
        # Go to next GPU
        let n=n+1
    done

    echo "Complete"; exit 0;
else
    echo "Error: Use ./oc_all.sh core [$OCCOREMIN-$OCCOREMAX] mem [$OCMEMMIN-$OCMEMMAX] pl [0-$PLMAX]"; 
    exit 1;
fi

