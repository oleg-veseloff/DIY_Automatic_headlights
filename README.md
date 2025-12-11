# DIY Automatic Headlights

This project is an automatic headlight system built with an ESP32, two light sensors (BH1750 and TEMT6000), an OLED display, and a 12V relay. The system checks ambient light and turns the headlights on or off. A delay is used to avoid fast changes when driving through short dark areas.

## Features
- Uses two sensors for more stable light readings  
- Shows data on a small OLED screen  
- Delay and hysteresis to stop light flickering  
- Safe control of a dedicated relay  
- Simple firmware for most ESP32 boards  

## Hardware
- ESP32 Dev Module  
- BH1750 digital light sensor  
- TEMT6000 analog sensor  
- SSD1306 OLED display  
- 1 Channel 5V High Level Trigger Relay Module
- 3.3V DC-DC converter  

## How It Works
1. The sensors measure the light level  
2. The ESP32 checks if it is dark or bright  
3. A delay prevents quick on/off switching  
4. The relay turns the headlights on or off  

## Firmware
The code is written in Arduino-style C++ for the ESP32.  
Upload it using the Arduino IDE with the ESP32 board package installed.

