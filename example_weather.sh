#!/bin/bash

# @brief Weatherdata from Yahoo! for the LCD
# @author Stephan Ruloff
#
# Source: https://developer.yahoo.com/weather/

LCD=./lcdialog

# Example:
#<yweather:location city="Dusseldorf" region="NW"   country="Germany"/>
#<yweather:units temperature="C" distance="km" pressure="mb" speed="km/h"/>
#<yweather:wind chill="18"   direction="220"   speed="12.87" />
#<yweather:atmosphere humidity="77"  visibility="9.99"  pressure="1015.92"  rising="0" />
#<yweather:astronomy sunrise="5:31 am"   sunset="9:42 pm"/>
#<yweather:condition  text="Mostly Cloudy"  code="28"  temp="18"  date="Tue, 15 Jul 2014 1:18 pm CEST" />
#<yweather:forecast day="Tue" date="15 Jul 2014" low="16" high="22" text="Mostly Cloudy" code="28" />
#<yweather:forecast day="Wed" date="16 Jul 2014" low="14" high="26" text="AM Showers" code="39" />
#<yweather:forecast day="Thu" date="17 Jul 2014" low="17" high="28" text="Partly Cloudy" code="30" />
#<yweather:forecast day="Fri" date="18 Jul 2014" low="19" high="31" text="Sunny" code="32" />
#<yweather:forecast day="Sat" date="19 Jul 2014" low="19" high="31" text="Mostly Sunny" code="34" />

# Search Yahoo! for your town.
WOEID=2442047

# c=Celsius, f=Fahrenheit
UNIT=c

# Name for the cache file of the weather data
CACHE_FILE=/tmp/weather.txt

# Refreshtime [s]
REFRESH=3600

if [ $# -eq 2 ]; then
	FONT=$1
	BGLIGHT=$2
else
	FONT=""
	BGLIGHT=0
fi

if [ -f $CACHE_FILE ] ; then
	AGE=$((`date +%s` - `stat -L --format %Y $CACHE_FILE`))
	#echo "AGE $AGE"
else
	AGE=$(($REFRESH + 1))
fi
$LCD --init --bglight $BGLIGHT --info "           Loading data ..."
if [ $AGE -gt $REFRESH ]; then
	wget -q -O $CACHE_FILE "http://weather.yahooapis.com/forecastrss?w=$WOEID&u=$UNIT"
fi

LINE=`cat $CACHE_FILE | grep yweather:condition`

[[ "$LINE" =~ (temp=\"([0-9\-]*)\") ]] && TEMP=${BASH_REMATCH[2]}
[[ "$LINE" =~ (code=\"([0-9]*)\") ]] && CODE=${BASH_REMATCH[2]}
[[ "$LINE" =~ (text=\"([a-zA-Z\(\) ]*)\") ]] && TEXT=${BASH_REMATCH[2]}


#echo "$TEMP C"
#echo "$TEXT"
$LCD $FONT --bglight $BGLIGHT --bgimg weathericons/$CODE.png:+90+0 --ok "$TEMP°C\n$TEXT"
