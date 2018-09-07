#include "ANEMOMETER.h"
#include "global.h"

/*  
 *  Function:       bool ANEMOMETER::setup() 
 *  Description:    setup for the anemometer
 */
bool ANEMOMETER::setup() {

    pinMode(analog_pin, INPUT);
    analogReadResolution(12);

    return true;

} // end of setup()

/*  
 *  Function:       bool ANEMOMETER::read() 
 *  Description:    read the anemometer
 */
bool ANEMOMETER::read() {

    if( read_windspeed() == true ) {
        return true;
    } else {
        return false;
    }

} // end of read()

/*  
 *  Function:       void ANEMOMETER::print_data() 
 *  Description:    print all data from anemometer
 */
void ANEMOMETER::print_data() {

    print_debug(data,                               // data where we store values
                time_data,                          // where we store time data
                ANEMOMETER_num_of_variables,           // number of sensor we use
                counter_col,                        // counter for coloumn index
                counter_row,                        // counter for row index
                counter_col_overflow);              // flag for coloumn counter overflow
 
} // end of print_data()

/*  
 *  Function:       bool ANEMOMETER::read_windspeed()
 *  Description:    read the windspeed
 */
bool ANEMOMETER::read_windspeed() {
    int an_data         = analogRead(analog_pin);

    // out of range
    if(an_data < 450 || an_data > 2600) {
        return false;
    }

    float an_converted  = mapf(an_data, 496, 2482, 0, 32.4);

    #ifdef debug
        serial_debug.print("ANEMOMETER (read_windspeed) - Wind Speed:");
        serial_debug.println(an_converted);
    #endif

    int int_an_converted = int(an_converted * 100);

    // updating the data into our data array
    update_16bit(data,                          // our data array
                time_data,                      // our time data array
                id_windspeed,                // temperature ID
                counter_col,                    // coloumn counter
                counter_row,                    // row counter
                counter_col_overflow,           // check if coloumn has overflow 
                ANEMOMETER_num_of_variables,       // number of variables
                int_an_converted);          // the value

    return true;

} // end of read_windspeed()