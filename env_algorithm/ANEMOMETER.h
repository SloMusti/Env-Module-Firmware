#ifndef ANEMOMETER_h
#define ANEMOMETER_h

#include <Arduino.h>

#define ANEMOMETER_num_of_variables 1

class ANEMOMETER
{
    public:
        // functions
        bool setup();
        bool read();
        void print_data();

        // variables
        int CAN_ID;

        // print_data
        uint8_t data[ANEMOMETER_num_of_variables][8][8];
        int time_data[8][8];

        // debug
        String name = "ANEMOMETER";

    private:

        // functions
        bool read_windspeed();

        // variables
        float current_windspeed                 = 0;
        int analog_pin                          = A0;

        int         counter_col                 = 0;
        int         counter_row                 = 0;
        bool        counter_col_overflow        = false;

        // id of value
        uint8_t id_windspeed                    = 0;
};   

#endif