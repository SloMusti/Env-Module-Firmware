// Modules
#include "can_module.h"

CAN_MODULE  module_CAN;

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

  // setup all modules
  device_setup(module_CAN.setup(), module_CAN.name);

  // setup all the sensors
  device_setup(sensor_L0.setup(), sensor_L0.name);                    // setup L0
  module_CAN.set_sensor_CAN_id(sensor_L0.CAN_ID, 1);                  // set the CAN ID to 1 (0x101)

  device_setup(sensor_TSL.setup(), sensor_TSL.name);                  // setup TSL2561
  module_CAN.set_sensor_CAN_id(sensor_TSL.CAN_ID, 2);                 // set the CAN ID to 2 (0x102)

  device_setup(sensor_BME.setup(), sensor_BME.name);                  // setup BME280
  module_CAN.set_sensor_CAN_id(sensor_BME.CAN_ID, 3);                 // set the CAN ID to 3 (0x103)

  device_setup(sensor_ANEMOMETER.setup(), sensor_ANEMOMETER.name);    // setup ANEMOMETER
  module_CAN.set_sensor_CAN_id(sensor_ANEMOMETER.CAN_ID, 4);          // set the CAN ID to 4 (0x104)

  device_setup(sensor_RAIN.setup(), sensor_RAIN.name);                // setup RAIN
  module_CAN.set_sensor_CAN_id(sensor_RAIN.CAN_ID, 5);                // set the CAN ID to 5 (0x105)

  device_setup(sensor_CO2.setup(), sensor_CO2.name);                  // setup CO2
  module_CAN.set_sensor_CAN_id(sensor_CO2.CAN_ID, 6);                 // set the CAN ID to 6 (0x106)

  device_setup(sensor_TDR.setup(), sensor_TDR.name);                  // setup TDR
  module_CAN.set_sensor_CAN_id(sensor_TDR.CAN_ID, 7);                 // set the CAN ID to 7 (0x107)

} // end of setup

void loop() {

  device_read(sensor_L0.read(), sensor_L0.name);
  sensor_L0.print_data();

  device_read(sensor_TSL.read(), sensor_TSL.name);
  sensor_TSL.print_data();

  device_read(sensor_BME.read(), sensor_BME.name);
  sensor_BME.print_data();

  device_read(sensor_ANEMOMETER.read(), sensor_ANEMOMETER.name);
  sensor_ANEMOMETER.print_data();

  device_read(sensor_RAIN.read(), sensor_RAIN.name);
  sensor_RAIN.print_data();

  device_read(sensor_CO2.read(), sensor_CO2.name);
  sensor_CO2.print_data();

  device_read(sensor_TDR.read(), sensor_TDR.name);
  sensor_TDR.print_data();

  delay(1000);
} // end of loop

/*
 *  Function:     void device_setup(bool status, String name)
 *  Description:  helper function for setting up device
 */
void device_setup(bool status, String device_name) {

  #ifdef debug
      serial_debug.print("device_setup() - device ");
      serial_debug.print(device_name);
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
} // end of device_setup

/*
 *  Function:     void sensor_read(bool status)
 *  Description:  helper function for reading the sensor
 */
void device_read(bool status, String device_name) {

  #ifdef debug
      serial_debug.print("device_read() - device ");
      serial_debug.print(device_name);
  #endif

  // if we get a false status (didnt read it good)
  if(!status) {
    #ifdef debug
      serial_debug.print("device_read() - device ");
      serial_debug.println("didn't read values properly!");
    #endif
  } else {
    #ifdef debug
      serial_debug.print("device_read() - device ");
      serial_debug.println("values read!");
    #endif
  }

} // end of device_read
