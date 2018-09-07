#include "BME.h"
#include "global.h"

#include <BME280.h>
#include <Wire.h>

BME280 _BME280(Wire, BME_I2C_ADDRESS);

/*
 *  Function:       bool BME::setup()
 *  Description:    setup for bme
 */
bool BME::setup() {
    if(_BME280.begin() < 0) {
        return false;
    } else {
        return true;
    }
} // end of setup()

/*
 *  Function:       bool BME::read()
 *  Description:    read all sensor data from BME
 */
bool BME::read() {

    if( read_pressure() == true &&
        read_temperature() == true &&
        read_humidity() == true         ) {
        
        return true;
    } else {
        return false;
    }
} // end of read()

/*
 *  Function:       bool BME::print_data()
 *  Description:    print all data
 */
void BME::print_data() {
    
    print_debug(data,                               // data where we store values
                time_data,                          // where we store time data
                BME280_num_of_variables,           // number of sensor we use
                counter_col,                        // counter for coloumn index
                counter_row,                        // counter for row index
                counter_col_overflow);              // flag for coloumn counter overflow
 
} // end of print_data()

/**************************************************************************************/

/*
 *  Function:       bool BME::read_pressure()
 *  Description:    read pressure data
 */
bool BME::read_pressure() {

    current_pressure = _BME280.getPressure_Pa();

    #ifdef debug
        serial_debug.print("BME (read_pressure) - Pressure:");
        serial_debug.println(current_pressure);
    #endif

    // updating the data into our data array
    update_16bit(data,                          // our data array
                time_data,                      // our time data array
                id_pressure,                // pressure ID
                counter_col,                    // coloumn counter
                counter_row,                    // row counter
                counter_col_overflow,           // check if coloumn has overflow 
                BME280_num_of_variables,       // number of variables
                current_pressure);          // the value

    return true;

}// end of read_pressure()

/*
 *  Function:       bool BME::read_temperature()
 *  Description:    read the temperature value
 */
bool BME::read_temperature() {

    current_temperature = _BME280.getTemperature_C();

    #ifdef debug
        serial_debug.print("BME (read_temperature) - Temperature:");
        serial_debug.println(current_temperature);
    #endif

    // updating the data into our data array
    update_16bit(data,                          // our data array
                time_data,                      // our time data array
                id_temperature,                // temperature ID
                counter_col,                    // coloumn counter
                counter_row,                    // row counter
                counter_col_overflow,           // check if coloumn has overflow 
                BME280_num_of_variables,       // number of variables
                current_temperature);          // the value

    return true;

} // end of read_temperature()

/*
 *  Function:       bool BME::read_humidity()
 *  Description:    read the humidity data
 */
bool BME::read_humidity() {

    current_humidity = _BME280.getHumidity_RH();

    #ifdef debug
        serial_debug.print("BME (read_humidity) - Humidity:");
        serial_debug.println(current_humidity);
    #endif

    // updating the data into our data array
    update_16bit(data,                          // our data array
                time_data,                      // our time data array
                id_humidity,                // humidity ID
                counter_col,                    // coloumn counter
                counter_row,                    // row counter
                counter_col_overflow,           // check if coloumn has overflow 
                BME280_num_of_variables,       // number of variables
                current_humidity);          // the value

    return true;
} // end of read_humidity()
