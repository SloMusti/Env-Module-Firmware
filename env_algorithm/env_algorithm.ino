// Sensor Includes
#include "L0.h"
#include "TSL.h"
#include "BME.h"
#include "ANEMOMETER.h"
#include "RAIN.h"
#include "CO2.h"
#include "TDR.h"

// Sensor objects
L0          sensor_L0;
TSL         sensor_TSL;
BME         sensor_BME;
ANEMOMETER  sensor_ANEMOMETER;
RAIN        sensor_RAIN;
CO2         sensor_CO2;
TDR         sensor_TDR;

// Extra Includes 
#include "global.h"

void setup() {

  // setup serial 
  serial_debug.begin(serial_debug_speed);

  // setup all the sensors
  sensor_setup(sensor_L0.setup(), sensor_L0.name);                    // setup L0
  sensor_setup(sensor_TSL.setup(), sensor_TSL.name);                  // setup TSL2561
  sensor_setup(sensor_BME.setup(), sensor_BME.name);                  // setup BME280
  sensor_setup(sensor_ANEMOMETER.setup(), sensor_ANEMOMETER.name);    // setup ANEMOMETER
  sensor_setup(sensor_RAIN.setup(), sensor_RAIN.name);                // setup RAIN
  sensor_setup(sensor_CO2.setup(), sensor_CO2.name);                  // setup CO2
  sensor_setup(sensor_TDR.setup(), sensor_TDR.name);                  // setup TDR

} // end of setup

void loop() {

  sensor_read(sensor_L0.read(), sensor_L0.name);
  sensor_L0.print_data();

  sensor_read(sensor_TSL.read(), sensor_TSL.name);
  sensor_TSL.print_data();

  sensor_read(sensor_BME.read(), sensor_BME.name);
  sensor_BME.print_data();

  sensor_read(sensor_ANEMOMETER.read(), sensor_ANEMOMETER.name);
  sensor_ANEMOMETER.print_data();

  sensor_read(sensor_RAIN.read(), sensor_RAIN.name);
  sensor_RAIN.print_data();

  sensor_read(sensor_CO2.read(), sensor_CO2.name);
  sensor_CO2.print_data();

  sensor_read(sensor_TDR.read(), sensor_TDR.name);
  sensor_TDR.print_data();

  delay(1000);
} // end of loop

/*
 *  Function:     void sensor_setup(bool status, String name)
 *  Description:  helper function for setting up sensors
 */
void sensor_setup(bool status, String sensor_name) {

  #ifdef debug
      serial_debug.print("sensor_setup() - sensor ");
      serial_debug.print(sensor_name);
  #endif

  // if we get a false status (not inited)
  if(!status) {

    #ifdef debug
      serial_debug.println(" not inited! CHECK WIRING! Stopping firmware...");
    #endif
 
    while(1);
  } else {

    #ifdef debug
      serial_debug.println(" inited!");
    #endif

  }
} // end of sensor_setup

/*
 *  Function:     void sensor_read(bool status)
 *  Description:  helper function for reading the sensor
 */
void sensor_read(bool status, String sensor_name) {

  #ifdef debug
      serial_debug.print("sensor_read() - sensor ");
      serial_debug.print(sensor_name);
  #endif

  // if we get a false status (didnt read it good)
  if(!status) {
    #ifdef debug
      serial_debug.println("L0 didn't read values properly!");
    #endif
  } else {
    #ifdef debug
      serial_debug.println("L0 values read!");
    #endif
  }

} // end of sensor_read
