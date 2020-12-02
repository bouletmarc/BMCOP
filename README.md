# BMCOP

The BMCOP is a Coil On Plug Project made with Arduino Nano V3 for Honda/Acura running on OBD1 ecus and with a distributor.

*This project are still in Developpement, prototyping it is completly at your own risk!*

# Introduction

The BMCOP need 2x Signal Inputs pins to acheive triggering all the four(4x) Coils. It require 1x ICM pulse signal (usually yellow/Green wire coming out of A20/A21 output pins on the ECU) and a 'replicated TDC' pulse signal coming from the FC1 header INSIDE the ecu. You'll have to get the signal from the 4th Pins of the FC1 Header counting from the top corner of the ECU. The FC1 Header are usually in the Top Right corner in a USDM OBD1 ECU.

You have to bring the ICM pulse (from A20/A21) ONLY to the Arduino and nothing else, you have to cut the Wire going to the Distributor. The tachometer won't display any RPM, this function wasn't YET implemented during the test. I have added an Output pin for 'RPM OUT' on the prototype board but the function on this pin aren't implemented, it was suposed to duplicate the ICM Signal in order to feed the signal back to the tachometer!

The Project (mostly due to schematic design) can only drive 5V coils for now (usually all D17, K20, K24 coils used for the project are 5V, it shouldn't be a big compromise!).

# Parts required to assembled the Prototype PCB

- 4x 1K Ohm 1/6W Resistor
- 4x 2N4148 Diode
- 1x Arduino Nano V3 - 5V - 16Mhz
- 1x 100nF (0.1uF) Capacitor

*If you want to Power the Device with 12V:*
- 1x 78M05
- 1x 1N4002 (1N4004, 2N4148 and others diodes above 20V rating will works just as fine for a replacement!)


# BMCOP V1.2 Schematic/Diagram
![alt tag](https://github.com/bouletmarc/BMCOP/blob/main/eagle_2020-12-02_09-09-23.png)


[Clic HERE to Check my others Products!]:<https://bmdevs.fwscheckout.com/>
