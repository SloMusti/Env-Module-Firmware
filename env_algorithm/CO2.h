#ifndef CO2_h
#define CO2_h

#include <Arduino.h>

#define CO2_num_of_variables    1

#define CO2_SERIAL              Serial1
#define CO2_SERIAL_BAUD         9600

class CO2 
{
    public:

        // functions
        bool    setup();
        bool    read();
        void    print_data();

        // variables

        // data
        uint8_t data[CO2_num_of_variables][8][8];
        int     time_data[8][8];

        // debug
        String name = "CO2";
        
    private:

        // functions
        bool    read_CO2();

        // variables
        int     current_co2                 = 0;

        int     counter_col                 = 0;
        int     counter_row                 = 0;
        bool    counter_col_overflow        = false;

        // id of values
        uint8_t id_co2                      = 0;


};

#endif