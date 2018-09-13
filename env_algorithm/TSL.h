#ifndef TSL_h
#define TSL_h

#include <Arduino.h>

#define TSL2561_num_of_variables 3

class TSL
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
        byte data[TSL2561_num_of_variables][8][8];
        byte     time_data[8][8];

        // debug
        String name = "TSL2561";
    private:

        // functions
        bool        read_visibility();
        bool        read_fullspectrum();
        bool        read_infrared();

        //variables
        uint16_t    current_visibility          = 0;
        uint16_t    current_fullspectrum        = 0;
        uint16_t    current_infrared            = 0;

        // id of values
        uint8_t     id_visibility               = 0;
        uint8_t     id_fullspectrum             = 1;
        uint8_t     id_infrared                 = 2;

};

#endif