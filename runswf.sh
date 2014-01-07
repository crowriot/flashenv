#!/bin/sh

export LD_PRELOAD=`pwd`/libpreflashenv.so
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`/libs/

# copy keymap.default to keymap.ini
cp --no-clobber keymap.default keymap.ini

./flashenv "$1"

# return to launcher if any
$2

