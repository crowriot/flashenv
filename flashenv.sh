#!/bin/sh

# backup old gameconfig.ini 
cp --no-clobber gameconfig.ini gameconfig.bak

# copy gameconfig.default to gameconfig.ini
cp --no-clobber gameconfig.default gameconfig.ini

./flashenvui 

