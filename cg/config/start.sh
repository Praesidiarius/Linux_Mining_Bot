#!/bin/bash

# Set Variables for Miner
set GPU_FORCE_64BIT_PTR 0
set GPU_MAX_HEAP_SIZE 100
set GPU_USE_SYNC_OBJECTS 1
set GPU_MAX_ALLOC_PERCENT 100
set GPU_SINGLE_ALLOC_PERCENT 100

# Get Rig Name
RIGNAME=`hostname`

#start gpuminer
