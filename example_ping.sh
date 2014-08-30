#!/bin/bash

# @brief Example: Ping Google
# @author Stephan Ruloff
# @date 13.07.2014

ping_it()
{
	./lcdialog --bglight 7 --info "          Ping ..."
	PINGOUT=`ping -c1 -w2 8.8.8.8`
	if [ $? -eq 0 ]; then
		OUT=`echo "$PINGOUT" | grep time= | sed 's/time=//g' | awk '{ print $7, $8 }'`
		#OUT="ALIVE!"
		./lcdialog --bglight 2 --ok "Alive\n$OUT"
	else
		OUT="Timeout"
		./lcdialog --bglight 1 --ok "$OUT"
	fi
}

while [ true ]; do

	./lcdialog --bglight 7 --yesno "Ping Google-DNS"
	if [ $? -eq 0 ] ; then
		ping_it
	else
		./lcdialog --clear --bglight 0
		exit 0
	fi

done