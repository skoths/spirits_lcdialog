#!/bin/bash
#
# @author Stephan Ruloff
# @date 17.11.2014
#
# @brief Basic volume control
#
# GPLv2

LCD=./lcdialog

$LCD --init
C=i
while [ true ] ; do

	PERCENT=0
	RAW=`amixer sget PCM | grep "%]"`
	[[ "$RAW" =~ (\[([0-9]*)%\]) ]] && PERCENT=${BASH_REMATCH[2]}
	PERCENT=`$LCD --bglight 7 --singlestep --percent "Volume" $PERCENT 2>&1`
	RET=$?
	if [ $RET -eq 0 ] ; then
		$LCD --bglight 0 --clear
		exit 0
	else 
		if [ $RET -eq 3 ] ; then
			#echo "Vol: $PERCENT %"
			amixer sset PCM $PERCENT% > /dev/nul
		fi
	fi

done

