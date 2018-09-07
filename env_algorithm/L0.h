#ifndef L0_h
#define L0_h

#include <Arduino.h>
#include <stdint.h>

#define L0_num_of_variables 2

class L0 
{
    public:

        // functions
        bool    setup();
        bool    read();
        void    print_data();

        // variables
        
        // data
        uint8_t data[L0_num_of_variables][8][8];
        int     time_data[8][8];

        // debug
        String  name = "L0";
    private:

        // functions
        bool    read_temperature();
        bool    read_VDD();

        // variables
        float   current_temperature     = 0;
        float   current_vdd             = 0;

        int     counter_col             = 0;
        int     counter_row             = 0;
        bool    counter_col_overflow    = false;

        // id of the values
        uint8_t id_temperature          = 0;
        uint8_t id_vdd                  = 1;

};

#endif