#ifndef TDR_h
#define TDR_h

#include <Arduino.h>

#define TDR_num_of_variables 5

#define TDR_SERIAL          Serial
#define TDR_SERIAL_BAUD     115200

class TDR
{
    public:

        // functions
        bool    setup();
        bool    read();
        void    print_data();
        bool    exec_timer();

        // variables
        int CAN_ID;
        int exec_time                   = 30 * 1000;
        unsigned long exec_timer_last   = 0;

        int         counter_col                 = 0;
        int         counter_row                 = 0;
        bool        counter_col_overflow        = false;


        // data
        byte data[TDR_num_of_variables][8][8];
        byte     time_data[8][8];

        // debug
        String name = "TDR";

    private:

        // functions
        bool    read_allTDR();

        // variables
        int         current_vol_w_content       = 0;
        int         current_soil_temp           = 0;
        int         current_soil_perm           = 0;
        int         current_soil_elec           = 0;
        int         current_other_data          = 0;

        // id of values
        uint8_t     id_vol_w_content            = 0;
        uint8_t     id_soil_temp                = 1;
        uint8_t     id_soil_perm                = 2;  
        uint8_t     id_soil_elec                = 3;
        uint8_t     id_other_data               = 4;


};

#endif