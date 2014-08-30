#!/bin/bash
#
# @author Stephan Ruloff
# @date July 2014
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; in version 2 only
# of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.


if ! [ -f lcdialog ] ; then 
	echo "No lcdialog!"
	exit 1;
fi

BGLIGHT=7
IP=10.0.0.1
NM=255.255.255.0
IDX=1

./lcdialog --init
while [ true ] ; do

	C=`./lcdialog $FONT --bglight $BGLIGHT --menuindex "$IDX" --menu "1" "Ping" "2" "Weather" \
		"3" "Sysinfo" "4" "Show alphabet" "5" "IPv4 address" 6 "Subnet mask" 7 Progress 8 Percent \
		"R" "Red" "G" "Green" "B" "Blue" "W" "White" \
		"S" "Font small" "T" "Font medium" "U" "Font large" "X" "Exit" 2>&1`

	case $C in	
	1)	./example_ping.sh
		;;	
	2)	./example_weather.sh "$FONT" "$BGLIGHT"
		;;
	3)	./example_sysinfo.sh
		;;
	4)	./lcdialog $FONT --bglight $BGLIGHT --info "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		./lcdialog --buttonwait
		;;
	5)	IP=`./lcdialog $FONT --bglight $BGLIGHT --ip "$IP" 2>&1`
		;;
	6)	NM=`./lcdialog $FONT --bglight $BGLIGHT --subnetmask "$NM" "Subnet mask" 2>&1`
		;;
	7)	./example_progress.sh
		;;
	8)	VOL=`./lcdialog $FONT --bglight $BGLIGHT --percent "Volume" 75 2>&1`
		;;
	R)	BGLIGHT=1
		;;
	G)	BGLIGHT=2
		;;
	B)	BGLIGHT=4
		;;
	W)	BGLIGHT=7
		;;
	S)	FONT=""
		;;
	T)	FONT=--font-medium
		;;
	U)	FONT=--font-large
		;;
	X)	./lcdialog --clear --bglight 0
		exit 0
		;;

	*)	echo "Error '$C' Result:$?"
		exit ;
		;;
	esac
	IDX=$C

done
