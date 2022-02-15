# power_mgmt_firmware

This is the Arduino firmware running aboard the ATmega328PB on the battery mgmt board. This firmware speaks a machine dialect over UART1 and a human dialect over UART0 to monitor the voltages present in the system and control the 74-series logic used to select shore or battery power. Both UARTs run at 9600 baud.

This firmware does not build using ROS catkin. To build, open the .ino symlink in the Arduino IDE and compile / flash using a FTDI cable attached to the UART0 pins on the power mgmt board. Note, you will also need the firmware_msgs repo cloned in the parent directory to have struct definitions available.

The power mgmt board does not have a crystal, instead using the internal RC oscillator running at 8MHz. Use the ATTiny SPI programmer to burn initial Optiboot bootloader - then, the standard Arduino tools can be used to flash updates via the serial port. 
