#!/bin/sh

export LD_PRELOAD=`pwd`/libpreflashenv.so
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`/libs/

./flashenv "$1"

# reset framebuffer (just to be sure ...)
ofbset -fb /dev/fb1 -pos 0 0 -size 0 0 -mem 0 -en 0

# return to launcher if any
$2

