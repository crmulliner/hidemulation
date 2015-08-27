#!/bin/bash

#
# Collin Mulliner <collin AT mulliner.org>
#

modprobe -r g_ether usb_f_ecm u_ether
modprobe usb_f_hid

cd /sys/kernel/config/
mkdir usb_gadget/g1
cd usb_gadget/g1
mkdir configs/c.1
mkdir functions/hid.usb0
echo 1 > functions/hid.usb0/protocol
echo 1 > functions/hid.usb0/subclass
echo 8 > functions/hid.usb0/report_length
echo -ne "\x05\x01\x09\x06\xA1\x01\x05\x07\x19\xE0\x29\xE7\x15\x00\x25\x01\x75\x01\x95\x08\x81\x02\x95\x01\x75\x08\x81\x03\x95\x05\x75\x01\x05\x08\x19\x01\x29\x05\x91\x02\x95\x01\x75\x03\x91\x03\x95\x06\x75\x08\x15\x00\x25\x65\x05\x07\x19\x00\x29\x65\x81\x00\xC0" > functions/hid.usb0/report_desc
mkdir strings/0x409
mkdir configs/c.1/strings/0x409
echo 0x6240 > idProduct
echo 0x046d > idVendor
echo 0x0100 > bcdDevice # v1.0.0
echo 0x0200 > bcdUSB # USB2
echo "fedcba9876543210" > strings/0x409/serialnumber
echo "Logitech" > strings/0x409/manufacturer
echo "Keyboard" > strings/0x409/product
echo "Conf1" > configs/c.1/strings/0x409/configuration
echo 120 > configs/c.1/MaxPower
ln -s functions/hid.usb0 configs/c.1
echo ci_hdrc.0 > UDC
