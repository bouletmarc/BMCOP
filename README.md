# BMCOP

The BMCOP is a Coil On Plug Project made with Arduino Nano V3 for Honda/Acura running on OBD1 ecus and with a distributor.

***This project are still in Developpement, prototyping it is completly at your own risk!*

# Introduction

The BMCOP need 2x Signal Inputs pins to acheive triggering all the four(4x) Coils. It require 1x ICM pulse signal (usually yellow/Green wire coming out of A20/A21 output pins on the ECU) and a 'replicated TDC' pulse signal coming from the FC1 header INSIDE the ecu. You'll have to get the signal from the 4th Pins of the FC1 Header counting from the top corner of the ECU. The FC1 Header are usually in the Top Right corner in a USDM OBD1 ECU.

You have to bring the ICM pulse (from A20/A21) ONLY to the Arduino and nothing else, you have to cut the Wire going to the Distributor. **The tachometer won't display any RPM**, this function wasn't YET implemented during the test. I have added an Output pin for 'RPM OUT' on the prototype board but the function on this pin aren't implemented in the arduino code, it was suposed to duplicate the ICM Signal in order to feed the signal back to the tachometer!

The Project (mostly due to schematic design) can only drive 5V coils for now (usually all D17, K20, K24 coils used for the project are 5V, it shouldn't be a big compromise!).

# Parts required to assembled the Prototype PCB

- 4x 1K Ohm 1/6W Resistor
- 4x 2N4148 Diode
- 1x Arduino Nano V3 - 5V - 16Mhz
- 1x 100nF (0.1uF) Capacitor

*If you want to Power the Device with 12V:*
- 1x 78M05
- 1x 1N4002 (1N4004, 2N4148 and others diodes above 20V rating will works just as fine for a replacement!)


# Possible Issues

While testing the project, I experienced miss fires happennning at High RPM but I couldn't finish the project in time before selling the only car I could have developped the project with.

**The Project is for use at your OWN COMPLETE RISK.

The Coils pack I been using during the tests, was D17 used coils and 1x of them had burning marks before I started the project. It's a possibility that the project works flawlessly and the issue I was experiencing during my tests are not at all related to a Programming OR Schematic design issue, that it was a faulty coil on my side BUT be aware that there is chances that you can experience High RPM miss firing happening if the issue wasn't this.

**If you acheive successfull results

I'll surely appreciate to get informed of the changes done the the programming or the schematic if there any, and if you would like, you could participate in the developpement of this project by subscribing to Github and yourself 'Push updates request' to this project! I am willing to share Open Source the project I ended up with before selling the only car I could have tested on, please be kindful and let's make this a working Open Source project together.

# BMCOP V1.2 Schematic/Diagram
![alt tag](https://github.com/bouletmarc/BMCOP/blob/main/eagle_2020-12-02_09-09-23.png)


[Clic HERE to Check my others Products!]:<https://bmdevs.fwscheckout.com/>
