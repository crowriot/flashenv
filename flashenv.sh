#!/bin/sh

FILE="`zenity --file-selection --filename='/media/' --file-filter='*.swf' --title='Select an SWF'`"
[ $? -eq 0 ] || exit 1;

./runswf.sh "$FILE"
