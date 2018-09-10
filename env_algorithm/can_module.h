#ifndef can_module_h
#define can_module_h

#include <Arduino.h>
#include <SPI.h>

#include <mcp_can.h>  
#include <mcp_can_dfs.h>

#define CAN_PIN_INT 4               // interrupt pin PB5
#define CAN_PIN_NSS 9               // chip-select PB12-NSS pin

#define CAN_MASTER_ID 0x001
#define CAN_ID      0x100

#define CAN_SPEED CAN_125KBPS       // CAN_125KBPS
#define CAN_MHZ   MCP_20MHZ          // MCP_16MHZ, MCP_20MHZ

class CAN_MODULE
{
    public:

        // functions
        bool setup();

        void set_sensor_CAN_id(int& ID, int num);

        // debug
        String name = "CAN";

    private:

};

#endif