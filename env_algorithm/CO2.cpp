#include "CO2.h"
#include "global.h"

#include <Arduino.h>

/*
 *  Function:       bool CO2::setup()
 *  Description:    setup the serial for the CO2
 */
bool CO2::setup() {
    CO2_SERIAL.begin(CO2_SERIAL_BAUD);

    return true;
} // end of setup()

/*
 *  Function:       bool CO2::read()
 *  Description:    read the sensors
 */
bool CO2::read() {
    if( read_CO2() == true) {
        return true;
    } else {
        return false;
    }
} // end of read()

/*
 *  Function:       void CO2::print_data()
 *  Description:    print the data
 */
void CO2::print_data() {

    print_debug(data,                               // data where we store values
                time_data,                          // where we store time data
                CO2_num_of_variables,           // number of sensor we use
                counter_col,                        // counter for coloumn index
                counter_row,                        // counter for row index
                counter_col_overflow);              // flag for coloumn counter overflow

} // end of print_data()

/*
 *  Function:    CO2::exec_timer()
 *  Description: execute read if the timer is right
 */
bool CO2::exec_timer() {

    if(millis() - exec_timer_last >= exec_time) {
        
        if(read()) {
            print_data();
            #ifdef debug
                serial_debug.println("CO2 (exec_timer) - executed read");
            #endif

            exec_timer_last = millis();

            return true;
        }
        return false;
        
    }

    return false;
} // end of exec_timer()

/*
 *  Function:       bool CO2:read_CO2()
 *  Description:    read the CO2 through serial
 */
bool CO2::read_CO2() {
    String recv_string = "";

    while(CO2_SERIAL.available() > 0) {

        int inChar = CO2_SERIAL.read();

        if(inChar == 'z' || inChar == 'Z') {
            recv_string = "";
        }

        if(isDigit(inChar)) {
            recv_string += (char)inChar;
        }

        if(inChar == '\n') {
            current_co2 = (recv_string.toInt() / 10);
        }

    }

    return true;
} // end of read_CO2

