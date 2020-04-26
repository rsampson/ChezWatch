# ChezWatch
Home security system using a pir sensor and multiple esp8266 microprocessors in esp-now mode for low power. It uses multiple transmitters based on the esp-01 and a single receiver/hub that could be based on any esp8266 device.

When the transitter pir sensor detects movement, it will enable the esp-01 to apply power briefly. In esp-now mode, the transmitter can forward a reading of its battery voltage to the receiver in under two seconds, after which it will go to sleep and the pir sensor will power it off completely in a few seconds. To further limit power, the red LED is removed from the esp-01 and the 3.3 volt regulator is removed from the pir sensor (as it will be connected directly to the 3 volts of two aaa cells). In this manner power drain is reduced enough so that the aaa batteries should last over a year.

The receiver could be programmed to do any number of things, but in our case, it will transmit the esp-01 battery voltage to the cloud service "ThingSpeak" (https://thingspeak.com/). ThingSpeak is a service that will record various IOT events that can be viewed on the web from anywhere in the world. 


