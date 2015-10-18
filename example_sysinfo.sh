#!/bin/bash
#
# @brief Simple Sysinfo
#
# @author Stephan Ruloff

LCD=./lcdialog

sysinfo()
{
	# Version 1
	CPU=`grep 'cpu ' /proc/stat | awk '{usage=($2+$4)*100/($2+$4+$5)} END {printf("%1.0f%%", usage)}'`
	# Version 2
	# CPU=`iostat | head -4 | tail -1 | awk '{print $1, $2, $3, $6}' | sed -e 's/ /:/g' | sed -e 's/,/./g'`

	# Version 1
	#TEMPCPU=`sudo /opt/vc/bin/vcgencmd measure_temp | cut -d '=' -f 2 | cut -d "'" -f 1`
	# Version 2
	if [ -f /sys/class/thermal/thermal_zone0/temp ] ; then
		TEMPCPU=`cat /sys/class/thermal/thermal_zone0/temp | awk '{t=$1/1000} END {printf("RPi %1.1f", t)}'`
	else
		TEMPCPU="BPi "`soctemp | awk '{print $2}' | sed -e 's/°C//g'`
	fi

	DISK=`df | grep rootfs | awk '{ print $5 }'`

	RAMTOTAL=`grep MemTotal /proc/meminfo | awk '{print $2}'`
	RAMFREE=`grep MemFree /proc/meminfo | awk '{print $2}'`

	RAM=`echo $((($RAMFREE*100) / $RAMTOTAL))`


	#echo "CPU: $CPU used"
	#echo "DISK: $DISK used"
	#echo "Temp: $TEMPCPU °C"
	#echo "RAM: $RAM% free"
	LINE="CPU:\t$CPU used\nDisk:\t$DISK used\nTemp:\t$TEMPCPU °C\nRAM:\t$RAM% free"

	$LCD --bglight 7 --info "$LINE"
}


$LCD --init
EXIT=0
while [ $EXIT -eq 0 ] ; do
	sysinfo
	$LCD --timeout 5 --buttonwait
	RET=$?
	if [ $RET -ne 4 ] ; then
		EXIT=1
	fi
done
