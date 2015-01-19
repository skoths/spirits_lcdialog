LCDialog
========

LCD version of dialog

Dialog is a program to create text-boxes in bash scripts. LCDialog is a tool for embedded graphic lcds, 
for example, on a Raspberry Pi. The graphic display is a DOGM 132x32 pixel LCD. GPIO's and a SPI will 
be used to control the output, the LED backlight and read the button's.
Because we are using only linux standard API for GPIO and SPI, the program should be compatible with other boards like
the beagle-board-black, banana pi or other.

http://www.rk-tech.org

https://github.com/rstephan/lcdialog/wiki

yesno
-----

Text longer then the available space will be omitted.

LCD:

![Mode yesno](http://www.rk-tech.org/github/lcdialog/images/yesno.png)
 
e.g.

    $ lcdialog --yesno "Text1\nText2\nText3"

ok
----

LCD:

![Mode ok](http://www.rk-tech.org/github/lcdialog/images/ok.png)

e.g.

    $ lcdialog --ok "Text1\nText2\nText3"


menu
----

  - Marker

LCD:

![Mode menu tag item](http://www.rk-tech.org/github/lcdialog/images/menu_tag_item.png)

e.g.

    $ lcdialog --menu "Tag1" "Item1" "Tag2" "Item2" "Tag3" "Item3" "Tag4" "Item4"


  - Marker w/ no tag

LCD:

![Mode menu item](http://www.rk-tech.org/github/lcdialog/images/menu_item.png)

e.g.
 
    $ lcdialog --notag --menu "Tag1" "Item1" "Tag2" "Item2" "Tag3" "Item3" "Tag4" "Item4"

 
ipv4
----

IP-adress input


LCD:

![Mode ipv4](http://www.rk-tech.org/github/lcdialog/images/ipv4.png)

e.g.

    $ lcdialog --ipv4 192.168.0.1 "Text1\nText2"

subnetmask
-------

Subnet mask input

Only the CIDR-Value can be set, so it is faster to enter the subnet mask.
The advantage is that you can only set a valid subnet mask, and nothing like e.g. 255.0.0.255

LCD:

![Mode subnet mask](http://www.rk-tech.org/github/lcdialog/images/subnetmask.png)

e.g.

    $ lcdialog --subnetmask 255.255.0.0 "Text1\nText2"

 
info
------

All four lines are for your text.

LCD:

![Mode info](http://www.rk-tech.org/github/lcdialog/images/info.png)
 
e.g.

    $ lcdialog --info "Text1\nText2\nText3\nText4"


intinput
--------

LCD:

![Mode integer input](http://www.rk-tech.org/github/lcdialog/images/intinput.png)

e.g.

    $ lcdialog --intinput "Text1\nText2" 64 128


progress
--------

LCD:

![Mode progress bar](http://www.rk-tech.org/github/lcdialog/images/progress.gif)

e.g.

    $ { sleep 1; echo 66; sleep 1; echo 100; } | lcdialog --progress "Initial text" 33


percent
-------

LCD:

![Mode percent input](http://www.rk-tech.org/github/lcdialog/images/percent.png)

e.g.

    $ lcdialog --percent 75


 
Example 
-------

```
$ ./lcdialog --yesno "Bla Bla Blub"
$ echo $?
```


Options
-------

```
 --init
	Each LCD needs a init-sequenze to set the hardware up.
	The LCD only needs to be initialized once it is powered, to avoid flickering between
	screens. The LCD only needs to be initialized once at the beginning of the script.
	
 --uninit
	Releases all used resources (GPIO and SPI).

 --clear
	As it suggests, the LCD will be cleared. All pixels will be switched off.

 --notag
	Doesn't show the tag in menu line.

 --stepwidth value
	Increment for "--percent", default is 5.

 --singlestep
	In mode "--percent" every change will output the value and exit with EXITCODE_EXTRA.
	
 --timeout <seconds>
	After the specified time the program will exit with EXITCODE_TIMEOUT.
	
 --bgimg <filename>[:+x+y]
	Set filename for background image.
	The optional offset at the end will position the image on the screen.
	Only positiv values are permitted.
	
	e.g. 
	for an x-offset of 30 an y-offset of 5
	... --bgimg bla.png:+30+5 ...
	
 --bglight <value>
	Sets the LED background-color as a 1-Bit color value.

	Bit-0: red

	Bit-1: green

	Bit-2: blue

	0=off, 7=all on (white).

	e.g. for blue and green ...  --bglight 6 ...
```
![Mode background color](http://www.rk-tech.org/github/lcdialog/images/color.gif)

```
 --font-medium
	Select medium size font (11-px high).

 --font-large
	Select large size font (23-px high).

	No option will set the default font (7-px high).
```	

Character-Set
-------------

Everyone uses Unicode, we don't. Our font-set takes 8-Bit to encode the characters.
But for convenience, all UTF-8 input will be automaticaly converted down to ISO-8859-1.

