#!/bin/bash 

#
# Collin Mulliner <collin AT mulliner.org>
#

echo 154 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio154/direction
chmod 666 /sys/class/gpio/gpio154/direction
chmod 666 /sys/class/gpio/gpio154/value

echo 155 > /sys/class/gpio/export
echo in > /sys/class/gpio/gpio155/direction
chmod 666 /sys/class/gpio/gpio155/direction
chmod 666 /sys/class/gpio/gpio155/value
