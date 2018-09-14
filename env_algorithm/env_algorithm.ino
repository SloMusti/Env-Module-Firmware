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

// helping variables
bool status_sent = false;

void sleep_devices() {

  #ifdef debug
    serial_debug.println("sleep_devices() - sleeping for 10s");
  #endif

  //module_CAN.setMode(MCP_SLEEP);
  // STM32L0
  STM32L0.stop(10 * 1000);

}

void setup() {
  // setup serial 
  serial_debug.begin(serial_debug_speed);
  serial_debug.println("test");
  // setup all modules
  device_setup(module_CAN.setup(), module_CAN.name, 0);

  
  module_CAN.set_sensor_CAN_id(sensor_L0.CAN_ID, 1);                  // set the CAN ID to 1 (0x101)
  module_CAN.set_sensor_CAN_id(sensor_TSL.CAN_ID, 2);                 // set the CAN ID to 2 (0x102)
  module_CAN.set_sensor_CAN_id(sensor_BME.CAN_ID, 3);                 // set the CAN ID to 3 (0x103)
  module_CAN.set_sensor_CAN_id(sensor_ANEMOMETER.CAN_ID, 4);          // set the CAN ID to 4 (0x104)
  module_CAN.set_sensor_CAN_id(sensor_RAIN.CAN_ID, 5);                // set the CAN ID to 5 (0x105)
  module_CAN.set_sensor_CAN_id(sensor_CO2.CAN_ID, 6);                 // set the CAN ID to 6 (0x106)
  module_CAN.set_sensor_CAN_id(sensor_TDR.CAN_ID, 7);                 // set the CAN ID to 7 (0x107)


  // setup all the sensors
  device_setup(sensor_L0.setup(), sensor_L0.name, module_CAN.get_sensor_CAN_id(sensor_L0.CAN_ID));                    // setup L0
  device_setup(sensor_TSL.setup(), sensor_TSL.name, module_CAN.get_sensor_CAN_id(sensor_TSL.CAN_ID));                  // setup TSL2561
  device_setup(sensor_BME.setup(), sensor_BME.name, module_CAN.get_sensor_CAN_id(sensor_BME.CAN_ID));                  // setup BME280
  device_setup(sensor_ANEMOMETER.setup(), sensor_ANEMOMETER.name, module_CAN.get_sensor_CAN_id(sensor_ANEMOMETER.CAN_ID));    // setup ANEMOMETER
  device_setup(sensor_RAIN.setup(), sensor_RAIN.name, module_CAN.get_sensor_CAN_id(sensor_RAIN.CAN_ID));                // setup RAIN
  device_setup(sensor_CO2.setup(), sensor_CO2.name, module_CAN.get_sensor_CAN_id(sensor_CO2.CAN_ID));                  // setup CO2
  device_setup(sensor_TDR.setup(), sensor_TDR.name, module_CAN.get_sensor_CAN_id(sensor_TDR.CAN_ID));                  // setup TDR

} // end of setup

void loop() {
  
  // EXEC sending
  if(module_CAN.return_local_send_data_int()) {

    module_CAN.execute_int_can = false;

    #ifdef debug
      serial_debug.print("loop() - CAN_RXID chosen (ID): ");
      serial_debug.println(module_CAN.return_CAN_RXID(), HEX);
    #endif

    switch(module_CAN.get_sensor_CAN_id(module_CAN.return_CAN_RXID())) {

      // L0
      case 1:
        // read sensor
        device_read(sensor_L0.read(), sensor_L0.name);
        sensor_L0.print_data();

        // send data
        status_sent = module_CAN.send_data(
          sensor_L0.data,
          sensor_L0.time_data,
          NUM_IN_ARRAY(sensor_L0.data),
          sensor_L0.counter_col,
          sensor_L0.counter_row,
          sensor_L0.counter_col_overflow
        );

        if(status_sent) { 

          #ifdef debug
            serial_debug.println("loop() - CAN message sent, clearing...");
          #endif

          // clear data
          clear_data(
            sensor_L0.data,
            sensor_L0.time_data,
            sensor_L0.counter_col,
            sensor_L0.counter_row,
            sensor_L0.counter_col_overflow
          );

        } else {
          #ifdef debug
            serial_debug.println("loop() - CAN message ERROR, not clearing...");
          #endif
        }

        break;

      // TSL
      case 2:
        // read sensor
        device_read(sensor_TSL.read(), sensor_TSL.name);
        sensor_TSL.print_data();

        // send data
        status_sent = module_CAN.send_data(
          sensor_TSL.data,
          sensor_TSL.time_data,
          NUM_IN_ARRAY(sensor_TSL.data),
          sensor_TSL.counter_col,
          sensor_TSL.counter_row,
          sensor_TSL.counter_col_overflow
        );

        if(status_sent) { 

          #ifdef debug
            serial_debug.println("loop() - CAN message sent, clearing...");
          #endif

          // clear data
          clear_data(
            sensor_TSL.data,
            sensor_TSL.time_data,
            sensor_TSL.counter_col,
            sensor_TSL.counter_row,
            sensor_TSL.counter_col_overflow
          );

        } else {
          #ifdef debug
            serial_debug.println("loop() - CAN message ERROR, not clearing...");
          #endif
        }
        break;

      // BME
      case 3:
        // read sensor
        device_read(sensor_BME.read(), sensor_BME.name);
        sensor_BME.print_data();

        // send data
        status_sent = module_CAN.send_data(
          sensor_BME.data,
          sensor_BME.time_data,
          NUM_IN_ARRAY(sensor_BME.data),
          sensor_BME.counter_col,
          sensor_BME.counter_row,
          sensor_BME.counter_col_overflow
        );

        if(status_sent) { 

          #ifdef debug
            serial_debug.println("loop() - CAN message sent, clearing...");
          #endif

          // clear data
          clear_data(
            sensor_BME.data,
            sensor_BME.time_data,
            sensor_BME.counter_col,
            sensor_BME.counter_row,
            sensor_BME.counter_col_overflow
          );

        } else {
          #ifdef debug
            serial_debug.println("loop() - CAN message ERROR, not clearing...");
          #endif
        }
        break;

      // ANEMOMETER
      case 4:
        // read sensor
        device_read(sensor_ANEMOMETER.read(), sensor_ANEMOMETER.name);
        sensor_ANEMOMETER.print_data();

        // send data
        status_sent = module_CAN.send_data(
          sensor_ANEMOMETER.data,
          sensor_ANEMOMETER.time_data,
          NUM_IN_ARRAY(sensor_ANEMOMETER.data),
          sensor_ANEMOMETER.counter_col,
          sensor_ANEMOMETER.counter_row,
          sensor_ANEMOMETER.counter_col_overflow
        );

        if(status_sent) { 

          #ifdef debug
            serial_debug.println("loop() - CAN message sent, clearing...");
          #endif

          // clear data
          clear_data(
            sensor_ANEMOMETER.data,
            sensor_ANEMOMETER.time_data,
            sensor_ANEMOMETER.counter_col,
            sensor_ANEMOMETER.counter_row,
            sensor_ANEMOMETER.counter_col_overflow
          );

        } else {
          #ifdef debug
            serial_debug.println("loop() - CAN message ERROR, not clearing...");
          #endif
        }
        break;

      // RAIN
      case 5:
        // read sensor
        device_read(sensor_RAIN.read(), sensor_RAIN.name);
        sensor_RAIN.print_data();

        // send data
        status_sent = module_CAN.send_data(
          sensor_RAIN.data,
          sensor_RAIN.time_data,
          NUM_IN_ARRAY(sensor_RAIN.data),
          sensor_RAIN.counter_col,
          sensor_RAIN.counter_row,
          sensor_RAIN.counter_col_overflow
        );

        if(status_sent) { 

          #ifdef debug
            serial_debug.println("loop() - CAN message sent, clearing...");
          #endif

          // clear data
          clear_data(
            sensor_RAIN.data,
            sensor_RAIN.time_data,
            sensor_RAIN.counter_col,
            sensor_RAIN.counter_row,
            sensor_RAIN.counter_col_overflow
          );

        } else {
          #ifdef debug
            serial_debug.println("loop() - CAN message ERROR, not clearing...");
          #endif
        }
        break;

      // CO2
      case 6:
        // read sensor
        device_read(sensor_CO2.read(), sensor_CO2.name);
        sensor_CO2.print_data();

        // send data
        status_sent = module_CAN.send_data(
          sensor_CO2.data,
          sensor_CO2.time_data,
          NUM_IN_ARRAY(sensor_CO2.data),
          sensor_CO2.counter_col,
          sensor_CO2.counter_row,
          sensor_CO2.counter_col_overflow
        );

        if(status_sent) { 

          #ifdef debug
            serial_debug.println("loop() - CAN message sent, clearing...");
          #endif

          // clear data
          clear_data(
            sensor_CO2.data,
            sensor_CO2.time_data,
            sensor_CO2.counter_col,
            sensor_CO2.counter_row,
            sensor_CO2.counter_col_overflow
          );

        } else {
          #ifdef debug
            serial_debug.println("loop() - CAN message ERROR, not clearing...");
          #endif
        }
        break;

      // TDR
      case 7:
        // read sensor
        device_read(sensor_TDR.read(), sensor_TDR.name);
        sensor_TDR.print_data();

        // send data
        status_sent = module_CAN.send_data(
          sensor_TDR.data,
          sensor_TDR.time_data,
          NUM_IN_ARRAY(sensor_TDR.data),
          sensor_TDR.counter_col,
          sensor_TDR.counter_row,
          sensor_TDR.counter_col_overflow
        );

        if(status_sent) { 

          #ifdef debug
            serial_debug.println("loop() - CAN message sent, clearing...");
          #endif

          // clear data
          clear_data(
            sensor_TDR.data,
            sensor_TDR.time_data,
            sensor_TDR.counter_col,
            sensor_TDR.counter_row,
            sensor_TDR.counter_col_overflow
          );

        } else {
          #ifdef debug
            serial_debug.println("loop() - CAN message ERROR, not clearing...");
          #endif
        }
        break;
    
    
    }

    module_CAN.execute_int_can = true;
    module_CAN.set_local_send_data_int(false);

  } else {
    if(module_CAN.return_local_send_data_int() == false && module_CAN.execute_int_can == true) {

      // sleep devices
      sleep_devices();

      if(module_CAN.return_local_send_data_int() == false && module_CAN.execute_int_can == true) {

        sensor_L0.exec_timer();
        sensor_TSL.exec_timer();
        sensor_BME.exec_timer();
        sensor_ANEMOMETER.exec_timer();
        sensor_RAIN.exec_timer();
        sensor_CO2.exec_timer();
        sensor_TDR.exec_timer();

      } // end of if
    } // end of if
  } // end of if
} // end of loop

/*
 *  Function:     void device_setup(bool status, String name)
 *  Description:  helper function for setting up device
 */
void device_setup(bool status, String device_name, int ID) {

  #ifdef debug
      serial_debug.print("device_setup() - device ");
      serial_debug.print(device_name);
  #endif

  // if we get a false status (not inited)
  if(!status) {

    #ifdef debug
      serial_debug.println(" not inited! CHECK WIRING! ");
    #endif

  } else {

    #ifdef debug
      serial_debug.println(" inited!");
    #endif

    // append it to the array of all inited
    if(ID != 0)
      module_CAN.append_available_sensor((unsigned char)(ID));


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
