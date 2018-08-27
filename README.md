# Env-Module Firmware
This repo contains the firmware for the [Environmental board](https://github.com/IRNAS/environmental-module-hardware) board.

## Hardware Support
- MCU:  CMWX1ZZABZ-091 LoRa®/Sigfox™ module (Murata)
- CAN: MCP25625 (integrated transiver)
- Multiple Feather compatible sensors

## Firmware logic
![Logic](https://raw.githubusercontent.com/silardg/Env-Module-Firmware/master/Docs/logic.png)

1. On start we read the resistors which are there to set the CAN ID
2. We init the CAN module (MCP25625)
3. We init the sensors we have defined
4. Send the CAN and MCU to sleep
5. IF we get an interrupt on CAN
    - read sensor data
    - store the sensor data
    - send back all avaible sensor data to master
5. IF the mcu wakes automatically after timer
    - read sensor data
    - store the sensor data
6. Go back to 4. 
