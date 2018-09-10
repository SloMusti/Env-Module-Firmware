#ifndef BME_h
#define BME_h

#include <Arduino.h>

#define BME280_num_of_variables 3
#define BME_I2C_ADDRESS 0x76

class BME
{
    public:

        // functions
        bool setup();
        bool read();
        void print_data();

        // variables
        int CAN_ID;

        // print_data
        byte data[BME280_num_of_variables][8][8];
        byte time_data[8][8];

        // debug
        String name = "BME280";

    private:

        // functions
        bool read_pressure();
        bool read_temperature();
        bool read_humidity();

        // variables
        float current_pressure                  = 0;
        float current_temperature               = 0;
        float current_humidity                  = 0;

        int         counter_col                 = 0;
        int         counter_row                 = 0;
        bool        counter_col_overflow        = false;

        // id of values
        uint8_t id_pressure                     = 0;
        uint8_t id_temperature                  = 1;
        uint8_t id_humidity                     = 2;

};

#endif