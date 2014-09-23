#!/bin/bash

# @author Stephan Ruloff
# @date 09.08.2014
# @licence GPLv2

LCD=./lcdialog

$LCD --init

{
sleep 1
echo "20"
sleep 1
echo "40"
sleep 1
echo "60"
sleep 1
echo "80"
sleep 1
echo "100"
} | $LCD --bglight 7 --progress "format c: /u" 10

sleep 1

{
sleep 1
echo -e "XXX\n20"
echo "rm -rf /root"
echo "XXX"
sleep .5
echo "30"
sleep .5
echo "40"
sleep .5
echo -e "XXX\n50"
echo "rm -rf /home/pi"
echo "XXX"
sleep .5
echo "60"
sleep .5
echo "70"
sleep .5
echo "80"
sleep .5
echo "90"
sleep .5
echo "100"
sleep 1
} | $LCD --bglight 7 --progress "Install Windows ..." 10
