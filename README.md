# BMCOP

The BMCOP is a Coil On Plug Project made with Arduino Nano V3 for Honda/Acura running on OBD1 ecus and with a distributor.

__*This project are still in Developpement, prototyping it is completly at your own risk!*__

# Introduction

The BMCOP need 2x Signal Inputs pins to acheive triggering all the four(4x) Coils. It require 1x ICM pulse signal (usually yellow/Green wire coming out of A20/A21 output pins on the ECU) and a pulse signal coming from the FC1 header INSIDE the ecu. You'll have to get the signal from the 3rd Pins of the FC1 Header counting from the top corner of the ECU. The FC1 Header are usually in the Top Right corner in a USDM OBD1 ECU.

# Connection wiring

- On the ECU, at the FC1 header location, connect the pin1 to 5V on the arduino
- On the ECU, at the FC1 header location, connect the pin3 to D2 on the arduino
- On the ECU, at the FC1 header location, connect the pin12 to GND on the arduino
- On the ECU, at the big connectors outputs, on the big connector A, CUT the ICM wire going to the body/engine bay and connect the wire coming from the ECU connector to D3 on the arduino
- On the Arduino, connect D8 with the ICM wire previously cut going to your body (to power the tachometer)
- On the Arduino, connect D4 with the Coil1 5v signal
- On the Arduino, connect D5 with the Coil2 5v signal
- On the Arduino, connect D6 with the Coil3 5v signal
- On the Arduino, connect D7 with the Coil4 5v signal

# Parts required to assembled the Prototype PCB

- 4x 1K Ohm 1/6W Resistor
- 4x 2N4148 Diode
- 1x Arduino Nano V3 - 5V - 16Mhz

(required ONLY for the v1.2 and older)
- 1x 100nF (0.1uF) Capacitor
*If you want to Power the Device with 12V:*
- 1x 78M05
- 1x 1N4002 (1N4004, 2N4148 and others diodes above 20V rating will works just as fine for a replacement!)

**In the Schematic 1N4002 take place of SS12**


# Possible Issues

__*The Project is for use at your OWN COMPLETE RISK, IT HAS NOT BEING TESTED/CONFIRMED*__

The project has been updated for the V1.3 boards, which now use a different signal for syncing coils (before it was using FC1 pin4, now i've updated the project to use FC1 pin3 signal). It has not been tested personnally since I don't have any cars/engines capable of testing the project on hands, but it should be more confident this time to works arround with it.

__*If you acheive successfull results*__

I'll surely appreciate to get informed of the changes done the the programming codes or the schematic if there is any, and if you would like, you could participate in the developpement of this project by subscribing to Github and yourself 'Push updates request' to this project! I am willing to share Open Source the project I ended up with before selling the only car I could have tested on, please be kindful and let's make this a working Open Source project together.

# BMCOP V1.3 Schematic/Diagram
![alt tag](https://github.com/bouletmarc/BMCOP/blob/main/eagle_1eO5gZZhDL.png)

# BMCOP V1.2 Schematic/Diagram
![alt tag](https://github.com/bouletmarc/BMCOP/blob/main/eagle_2020-12-02_09-09-23.png)


[Clic HERE to Check my others Products!]:<https://bmdevs.fwscheckout.com/>
