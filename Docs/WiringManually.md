# Wiring manually
- TSL2561 & BME280
    - connect VDD to +3.3V
    - connect GND to GND
    - connect SCL to SCL
    - connect SDA to SDA
- Wind sensor
    - brown wire to 12V
    - black wire to GND
    - Blue wire to PA0
- Rain sensor
    - PWR1 to 12V
    - PWR2 to GND
    - NO as showed here: ![image](http://static.cactus.io/img/hookups/circuits/hydreon-rg-11-hookup-circuit.jpg), where the digital  pin is PA10
    - COM to GND
    
## Raspberry Pi wiring
    - follow this to connect the MCP2515 module
        RPi Pin    RPi Label     CAN Module
        01---------3V3-----------VCC
        02---------5V------------extra soldered wire
        06---------GND-----------GND
        19---------GPIO10--------MOSI (SI)
        21---------GPIO9---------MISO (SO)
        22---------GPIO12--------INT - BCM12
        23---------GPIO11--------SCK
        24---------GPIO8---------CS
            - 
