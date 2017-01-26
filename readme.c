/*
 * In this sketch Arduino reads temperature with a simple two wire thermistor, 
 * records data to SDCard and display data at Nokia 5110 LCD using JeeLib Low power 
 * functions library to save on Arduino Power consumption
 * 
 * Creator: k3ntinhu@gmail.com
 * Date: 23 Jan 2017 
 * Update: 26 Jan 2017
 * 
 * 
--> SD Card Reader 
  The circuit:
   SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)

--> LCD5110
  This program requires a Nokia 5110 LCD module.
  It is assumed that the LCD module is connected to
      5 - CLK/SCK   - Pin 9
      4 - DIN/MOSI  - Pin 3
      3 - DC        - Pin 5
      1 - RST       - Pin 7
      2 - CE/CS     - Pin 6

--> BATTERY VOLTAGE
displays the voltage on the serial monitor and/or LCD shield
works with 3.3volt and 5volt Arduinos
uses the internal 1.1volt reference

IMPORTANT:
Arduino Vin --> 150k resistor <-- Analog pin 5 
Analog pin 5 --> 10k resistor <-- Ground
optional 100n capacitor from Analog pin to ground for stable readings


--> TWO WIRE THERMISTOR CONNECTIONS
Vcc <-- Thermistor1 | Thermistor 2 --> 10K resistor --> Ground


*/

