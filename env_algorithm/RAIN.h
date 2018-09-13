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
        bool    exec_timer();

        // variables
        int CAN_ID;
        int exec_time                   = 60 * 1000;
        unsigned long exec_timer_last   = 0;

        int         counter_col                 = 0;
        int         counter_row                 = 0;
        bool        counter_col_overflow        = false;

        // data
        byte data[RAIN_num_of_variables][8][8];
        byte     time_data[8][8];

        // debug
        String  name = "RAIN";

    private:

        // functions
        bool        read_raincount();

        // variables 
        int         pin_rain                    = 5;

        // id of values
        uint8_t     id_raincount                = 0;

};

#endif