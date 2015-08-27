# hidemulation

string2hid : takes a string and "types it" using /dev/hidgX (should work on any device that offers a usb hid gadget) 

usbarmory specifics:

hidonly.sh : switches the usbarmory to be usb hid gadget

hidnet.sh : switches the usbarmory to be a usb hid and usb ethernet gadget

button_setup.sh : switches pin 3 and 4 to in and out

button.sh : checks if pin 3 and 4 are connected
