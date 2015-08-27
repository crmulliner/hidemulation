#!/bin/bash 

#
# Collin Mulliner <collin AT mulliner.org>
#

echo out > /sys/class/gpio/gpio154/direction
echo in > /sys/class/gpio/gpio155/direction
echo 1 > /sys/class/gpio/gpio154/value

value=1
counter=0
while [ $value -eq "1" ]; do
echo 0 > /sys/class/gpio/gpio154/value
value=`cat /sys/class/gpio/gpio155/value` 
if [ "${value}" == "0" ]; then 
 #echo "button press"
 exit 0
 break
#else
# echo "."
fi 
sleep 1;
counter=$((counter + 1))
if [ $counter -eq 10 ]; then
 #echo "button expired"
 break
fi
done
exit 1 
