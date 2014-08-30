#!/bin/bash

#Change the next lines if necessary
squeeze_host=192.168.10.28
squeeze_port=9090
BGLIGHT=7	#7=white
IDX=1
VOL=`echo mixer volume ? | nc -q 1 $squeeze_host $squeeze_port | awk '{print $4}' | sed -e 's/%20/ /g'`


if ! [ -f lcdialog ] ; then 
	echo "No lcdialog!"
	exit 1;
fi

./lcdialog --init

while [ true ] ; do

	C=`./lcdialog --bglight $BGLIGHT --menuindex "$IDX" --notag --font-medium --menu "A" "Current Track" \
	"B" $'\x0C'$'\x0D'" Play/Pause" "C" $'\x10'" Stop" "D" $'\x0F'"Next Track" "E" $'\x0E'" Previous Track" \
	"F" $'\x13'" Mute" "G" $'\x12'" Volume $VOL" "H" "Info" "X" "Exit" 2>&1`
	case $C in
	#Show the "Current Track"
	A)	Title=`echo title ? | nc -q 1 $squeeze_host $squeeze_port | awk '{print $3}' | sed -e 's/%20/ /g'`
		Artist=`echo artist ? | nc -q 1 $squeeze_host $squeeze_port | awk '{print $3}' | sed -e 's/%20/ /g'`
		Album=`echo album ? | nc -q 1 $squeeze_host $squeeze_port | awk '{print $3}' | sed -e 's/%20/ /g'`
		Genre=`echo genre ? | nc -q 1 $squeeze_host $squeeze_port | awk '{print $3}' | sed -e 's/%20/ /g'`
		
		./lcdialog --bglight $BGLIGHT --info "$Title\n$Artist\n$Album\n$Genre"
		./lcdialog --keywait
		;;
	#Play/Pause
	B)	echo pause | nc -q 1 $squeeze_host $squeeze_port
		;;
	#Stop
	C)	echo stop | nc -q 1 $squeeze_host $squeeze_port
		;;
	#Next Track
	D)	echo playlist index +1 | nc -q 1 $squeeze_host $squeeze_port
		;;
	#Previous Track
	E)	echo playlist index -1 | nc -q 1 $squeeze_host $squeeze_port
		;;
	#Mute
	F)	echo mixer muting | nc -q 1 $squeeze_host $squeeze_port
		;;
	G)	VOL=`./lcdialog $FONT --bglight $BGLIGHT --stepwidth 3 --percent "Volume" $VOL 2>&1`
		echo mixer volume $VOL | nc -q 1 $squeeze_host $squeeze_port
		;;
	#Info
	H)	Name=`echo name ? | nc -q 1 $squeeze_host $squeeze_port | awk '{print $3}' | sed -e 's/%20/ /g'`
		IP=`/sbin/ifconfig eth0 | grep Bcast | awk '{ print $2 }' | awk -F: '{ print $2 }'`;
		TEMPCPU=`sudo /opt/vc/bin/vcgencmd measure_temp | cut -d '=' -f 2 | cut -d "'" -f 1`
		
		./lcdialog --bglight $BGLIGHT --info "$Name\nIP:$IP\nTemp:$TEMPCPU"
		./lcdialog --keywait
		;;			
	#Exit
	X)	./lcdialog --clear --bglight 0
		exit 0;
		;;
		
	*)
		echo ""
		;;
	esac
	IDX=$C

done
