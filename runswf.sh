#!/bin/sh

export LD_PRELOAD=`pwd`/libpreflashenv.so
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`/libs/

# copy gameconfig.default to gameconfig.ini
cp --no-clobber gameconfig.default gameconfig.ini

./flashenv "$1"

# return to launcher if any
$2

