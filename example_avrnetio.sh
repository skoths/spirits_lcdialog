#!/bin/bash

# @brief Pollin AVR-NET-IO
# @author Stephan Ruloff
# @date 27.08.2014
# @license GPLv2


LCD=./lcdialog
Value=(0 0 0 0 0 0 0 0 0)
IDX=1
AVR_IP=192.168.0.90

SetBit()
{
	if [ ${Value[$1]} -eq 0 ] ; then
		Value[$1]=1
	else
		Value[$1]=0
	fi
	#echo "SETPORT $1.${Value[$1]}"
	echo "SETPORT $1.${Value[$1]}" > /dev/tcp/$AVR_IP/50290
}

$LCD --init
while [ true ] ; do
	C=`$LCD --menuindex $IDX --bglight 7 --menu 1 ${Value[1]} 2 ${Value[2]} 3 ${Value[3]} 4 ${Value[4]} \
	5 ${Value[5]} 6 ${Value[6]} 7 ${Value[7]} 8 ${Value[8]} x Exit 2>&1`

	case $C in
	1 | 2 | 3 | 4 | 5 | 6 | 7 | 8)
		SetBit $C
		;;
	x)	./lcdialog --clear --bglight 0
	        exit 0
	        ;;
	*)	echo "Error" ; exit 1
		;;
	esac	
	IDX=$C
done