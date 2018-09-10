#ifndef RAIN_h
#define RAIN_h

#include <Arduino.h>
#include <STM32L0.h>

#define RAIN_num_of_variables 1

class RAIN
{
    public:

        // functions
        bool    setup();
        bool    read();
        void    print_data();

        // variables
        int CAN_ID;

        // data
        uint8_t data[RAIN_num_of_variables][8][8];
        int     time_data[8][8];

        // debug
        String  name = "RAIN";

    private:

        // functions
        bool        read_raincount();

        // variables 
        int         pin_rain                    = 5;

        // counter
        int         counter_col                 = 0;
        int         counter_row                 = 0;
        bool        counter_col_overflow        = false;

        // id of values
        uint8_t     id_raincount                = 0;

};

#endif