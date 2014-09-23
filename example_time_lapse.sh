#!/bin/bash

# @author Stephan Ruloff
# @date 11.08.2014
# @licence GPLv2

LCD=./lcdialog

$LCD --init

pidof raspistill > /dev/nul 2>&1
if [ $? -eq 0 ] ; then
	$LCD --bglight 7 --ok "raspistill is running\nplease wait and try again later."
	exit 1
fi

$LCD --yesno "Taking time lapse pictures?"

if [ $? -eq 0 ] ; then
	TIME_TOTAL_MIN=`$LCD --bglight 7 --intinput "How long (in minutes)" 1 1440 2>&1`
	if [ $? -eq 0 ] ; then
		TIME_BETWEEN_SEC=`$LCD --bglight 7 --intinput "Every (in seconds)" 1 60 2>&1`
		if [ $? -eq 0 ] ; then
			TIME_TOTAL_MS=$(($TIME_TOTAL_MIN * 60000))
			TIME_BETWEEN_MS=$(($TIME_BETWEEN_SEC * 1000))
			echo $TIME_TOTAL_MS -tl $TIME_BETWEEN_MS
			raspistill -o a%04d.jpg -t $TIME_TOTAL_MS -tl $TIME_BETWEEN_MS &
		fi
	fi
fi
$LCD --bglight 0 --clear
