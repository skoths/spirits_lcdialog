#!/bin/bash
#
# @author Stephan Ruloff
# @date 03.08.2014
#
# GPLv2

LCD=./lcdialog

# Format: tag name url/file/playlist
STATION_FILE=example_stations.txt

hash mpg123 > /dev/null 2>&1 || {
	echo "Need mpg123 !"
	echo "Try this:"
	echo "sudo aptitude install mpg123"
	exit 1
}

mp3_stop()
{
	pidof mpg123 > /dev/null 2>&1
	if [ $? -eq 0 ] ; then
		$LCD --info "Stopping audio ..."
		killall mpg123 > /dev/null 2>&1
		sleep 1
	fi
}

vol_up()
{
	amixer sset PCM 500+ > /dev/null
}

vol_down()
{	
	amixer sset PCM 500- > /dev/null
}

vol()
{
	PERCENT=0
	RAW=`amixer sget PCM | grep "%]"`
	[[ "$RAW" =~ (\[([0-9]*)%\]) ]] && PERCENT=${BASH_REMATCH[2]}
	PERCENT=`$LCD --bglight 7 --percent "Volume" $PERCENT 2>&1`
	amixer sset PCM $PERCENT% > /dev/nul
}

$LCD --init
C=i
while [ true ] ; do
	pidof mpg123 > /dev/null 2>&1
	if [ $? -eq 0 ] ; then
		INFO="Status: Playing"
	else
		INFO="Status: Stopped"
	fi
	OPTIONS=`cat $STATION_FILE | awk '{print $1, $2}'`
	OPTIONS="$OPTIONS s Stop u Vol-Up d Vol-Down v Volume x Exit" 
	#echo $OPTIONS
	C=`$LCD --bglight 7 --notag --menuindex "$C" --menu i "$INFO" $OPTIONS 2>&1`
	
	STATION=`egrep ^$C $STATION_FILE`
	if [ $? -eq 0 ] ; then
		mp3_stop
		NAME=`echo $STATION | awk '{print $2}'`
		URL=`echo $STATION | awk '{print $3}'`
		$LCD --info "Starting MP3 ...\n$NAME"
		#echo "Url: $URL"
		mpg123 -b 1024 $URL > /dev/null 2>&1 &
		sleep 1
	else
		case $C in
		s)	mp3_stop
			;;
		u)	vol_up
			;;
		d)	vol_down
			;;
		v)	vol
			;;
		i)	# do nothing
			;;

		* | x)	mp3_stop
			$LCD --bglight 0 --clear
			exit 0
			;;
		esac
	fi
	
done
