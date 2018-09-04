

// SENSOR DEFINES
#define L0_ID                       0                 // STM32L0 
#define L0_CANID                    0x101             // the ID through CAN
#define L0_VAR_NUM                  2                 // we have 2 variables

#define TSL2561_ID                  1
#define TSL2561_CANID               0x102             // the CAN ID of the sensor
#define TSL2561_VAR_NUM             3                 // we have 3 variables

#define BME280_ID                   2                 // ID is 2
#define BME280_CANID                0x103             // the CAN ID of the sensor
#define BME280_VAR_NUM              3                 // we have 3 variables

#define ANEMOMETER_ID               3                 // ID is 3
#define ANEMOMETER_CANID            0x104             // the CAN ID of the sensor
#define ANEMOMETER_VAR_NUM          1                 // we have 1 variable

#define RAIN_ID                     4                 // ID is 4
#define RAIN_CANID                  0x105             // the CAN ID of the sensor
#define RAIN_BUCKET_SIZE            0.01              // set by the dip switch (use 0.01mm sensitive)
#define RAIN_VAR_NUM                1                 // we have 1 variable

#define CO2_ID                      5                 // ID is 5
#define CO2_CANID                   0x106             // the CAN ID of the sensor
#define CO2_VAR_NUM                 1                 // we have 1 variable

#define TDR_ID                      6
#define TDR_CANID                   0x107
#define TDR_VAR_NUM                 4

/*
#define BQ34Z100_ID                 5                 // ID is 3
#define BQ34Z100_CANID              0x106             // the CAN ID of the sensor
#define BQ34Z100_VAR_NUM            4                 // we have 4 variables
#define BQ34Z100_SERIESCELLS        3                 // example value CHANGE IT!   Number of series Batteries
#define BQ34Z100_CELLCAPACITY       8000              // example value CHANGE IT!   Battery capacity in mAh
#define BQ34Z100_PACKVOLTAGE        12369             // example value CHANGE IT!   Current voltage on the pack in mV (eg 12.369V)
#define BQ34Z100_APPLIEDCURRENT     1000              // example value CHANGE IT!   Current being applied to the pack for currentShunt Cal in mA (must be > 200mA)
*/

const int number_of_sensors = 7;    // how many sensors are there

 
// EXTRA DEFINES
#define debug                       // if defined debug will show on serial       
#define TIMER_SECOND 5              // every TIMER_SECOND it wakes up

// PINS
#define ID0_PIN 6  // PB2
#define ID1_PIN A0 // PA0
#define ID2_PIN A3 // PA4
 
// MCU (sleep)
#include <STM32L0.h>    
    
// SERIAL
#define serial       Serial        // the serial we are using for debug
#define SERIAL_SPEED 115200         // the baud

// CAN MODULE
#include <mcp_can.h>  
#include <mcp_can_dfs.h>
#include <SPI.h>

#define CAN_PIN_INT 4               // interrupt pin PB5
#define CAN_PIN_NSS 9               // chip-select PB12-NSS pin

#define CAN_MASTER_ID 0x001
#define CAN_ID      0x100

#define CAN_SPEED CAN_500KBPS       // CAN_125KBPS
#define CAN_MHZ   MCP_8MHZ          // MCP_16MHZ, MCP_20MHZ

MCP_CAN CAN_BUS(CAN_PIN_NSS);       // can bus object

bool send_via_int = false;          // flag to send via the interrupt
bool exec_int_can = true;           // execute the interrupt can
bool not_my_id = false;
       
long unsigned int CAN_RXID;                   // used by ISR_CAN

// DATA
// store sensor data
// data_x[      sensors    ][vars][x*8bits][8 bits]
byte data[number_of_sensors][  4 ][   8   ][  8   ];  // number of sensors * variables * 64 bit
bool init_worked[number_of_sensors];                  // check if it has inited correctly 

int  data_index_coloumn[number_of_sensors];   // coloumn array
int  data_index_row[number_of_sensors];       // row array
bool data_coloumn_max[number_of_sensors];     // max coloumn value 

int shifted_first;                            // shifted first value
int shifted_second;                           // shifted second value
int shifted_third;                            // shifted third value
int shifted_fourth;                           // shifted fourth value
int old_row_index;                            // to store the old row index

// SENSORS
#include <Wire.h>

/**********************************************************************************/
//                               LUX SENSOR TSL2561                               //
#include "TSL2561.h" 
// The address will be different depending on whether you let
// the ADDR pin float (addr 0x39), or tie it to ground or vcc. In those cases
// use TSL2561_ADDR_LOW (0x29) or TSL2561_ADDR_HIGH (0x49) respectively
TSL2561 _TSL2561(TSL2561_ADDR_FLOAT); 

/**********************************************************************************/
//                                      BME280                                    //
#include "BME280.h"
/* A BME280 object with I2C address 0x76 (SDO to GND) */
BME280 _BME280(Wire,0x77);

/**********************************************************************************/
//                                     Anemometer                                 //
#define _ANEMOMETER_PIN A0

/**********************************************************************************/
//                                     Rain sensor                                //
#define _RAIN_PIN   5

volatile unsigned long rain_tipCount;        // bucket tip counter used in interrupt routine
volatile unsigned long rain_ContactTime;     // Timer to manage any contact bounce in interrupt routine

float rain_totalRainfall;                    // total amount of rainfall detected 

/**********************************************************************************/
//                                      CO2                                       //

#define CO2_serial          Serial1
#define CO2_serial_baud     9600

/**********************************************************************************/
//                                     TDR                                        //

#define TDR_serial          Serial
#define TDR_serial_baud     115200

/**********************************************************************************/
//                                      BQ34Z100                                  //
/*
#include "bq34z100.h"
bq34z100 _BQ34Z100;
*/


/*
 *  Function: mapf(float x, float in_min, float in_max, float out_min, float out_max)
 *  Description: map function with floats
 *  Paramter: same as map but with floats
 */
float mapf(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}



/*
 *  Function: void print_data()
 *  Description: print data fom data array
 *  Parameter: x
 */
void print_data() {
  
  serial.println("************************************************");
  serial.println("SENSOR L0");
  for(int j=0; j< L0_VAR_NUM; j++) {
    serial.print("VAR NUM: "); serial.println(j);
    for(int d=0; d<8; d++) {
      serial.print("DATA[");serial.print(d);serial.print("]: ");
      for(int c=0; c<8; c++) {
        serial.print(data[0][j][d][c]);serial.print("|");
      }
    }
  }
  serial.println();
  
  serial.println("SENSOR TSL2561");
  for(int j=0; j< TSL2561_VAR_NUM; j++) {
    serial.print("VAR NUM: "); serial.println(j);
    for(int d=0; d<8; d++) {
      serial.print("DATA[");serial.print(d);serial.print("]: ");
      for(int c=0; c<8; c++) {
        serial.print(data[1][j][d][c]);serial.print("|");
      }
    }
  }
  serial.println();
  
  serial.println("SENSOR BME280");
  for(int j=0; j<BME280_VAR_NUM; j++) {
    serial.print("VAR NUM: "); serial.println(j);
    for(int d=0; d<8; d++) {
      serial.print("DATA[");serial.print(d);serial.print("]: ");
      for(int c=0; c<8; c++) {
        serial.print(data[2][j][d][c]);serial.print("|");
      }
    }
  }
  serial.println();
  
  serial.println("SENSOR ANEMOMETER");
  for(int j=0; j<ANEMOMETER_VAR_NUM; j++) {
    serial.print("VAR NUM: "); serial.println(j);
    for(int d=0; d<8; d++) {
      serial.print("DATA[");serial.print(d);serial.print("]: ");
      for(int c=0; c<8; c++) {
        serial.print(data[3][j][d][c]);serial.print("|");
      }
    }
  }
  serial.println();

  serial.println("SENSOR RAIN");
  for(int j=0; j<RAIN_VAR_NUM; j++) {
    serial.print("VAR NUM: "); serial.println(j);
    for(int d=0; d<8; d++) {
      serial.print("DATA[");serial.print(d);serial.print("]: ");
      for(int c=0; c<8; c++) {
        serial.print(data[4][j][d][c]);serial.print("|");
      }
    }
  }
  serial.println();

  serial.println("SENSOR CO2");
  for(int j=0; j<CO2_VAR_NUM; j++) {
    serial.print("VAR NUM: "); serial.println(j);
    for(int d=0; d<8; d++) {
      serial.print("DATA[");serial.print(d);serial.print("]: ");
      for(int c=0; c<8; c++) {
        serial.print(data[5][j][d][c]);serial.print("|");
      }
    }
  }
  serial.println();

  serial.println("SENSOR TDR");
  for(int j=0; j<CO2_VAR_NUM; j++) {
    serial.print("VAR NUM: "); serial.println(j);
    for(int d=0; d<8; d++) {
      serial.print("DATA[");serial.print(d);serial.print("]: ");
      for(int c=0; c<8; c++) {
        serial.print(data[6][j][d][c]);serial.print("|");
      }
    }
  }
  serial.println();

  /*
  for(int i=0; i<number_of_sensors; i++) {
  serial.print("* SENSOR ");serial.print(i);serial.println(":                                    *");
    for(int j=0; j<4; j++) {
  serial.print("*    VAR ");serial.print(j);serial.println(":                                    *");
      for(int d=0; d<8; d++) {
  serial.print("*      DATA ");serial.print(d);serial.print(" :");
        for(int c=0; c<8; c++) {
  serial.print(data[i][j][d][c]);serial.print("|");
        }
        serial.print("                *");serial.println();
      }
    }
  }*/
  serial.println("************************************************");
}

/*
 *  Function:    void init_sensor(int num)
 *  Parameter:   int num - the number of the sensor
 *  Description: Choose your sensor by `num` and your sensor gets inited
 */
bool init_sensor(int num) {

  #ifdef debug
    serial.print("void select_sensor(int num) - Selecting sensor number: "); serial.print(num); serial.println();
  #endif

  
                     
  if(num == L0_ID) {
    #ifdef debug
      serial.println("void init_sensor(int num) - L0 MCU inited");
    #endif 

    // set the init var to true
    init_worked[L0_ID] = true;
    return true;
  } else if(num == TSL2561_ID) {

    // begin the communication for tsl2561
    if(_TSL2561.begin()) {
      #ifdef debug
        serial.println("void init_sensor(int num) - TSL2561 inited");
      #endif
    } else {
      #ifdef debug
        serial.println("void init_sensor(int num) - TSL2561 not inited!!");
      #endif
      init_worked[TSL2561_ID] = false;
      return false;
    }

    //_TSL2561.setGain(TSL2561_GAIN_0X);         // set no gain (for bright situtations)
    _TSL2561.setGain(TSL2561_GAIN_16X);      // set 16x gain (for dim situations)

    _TSL2561.setTiming(TSL2561_INTEGRATIONTIME_13MS);  // shortest integration time (bright light)
    //_TSL2561.setTiming(TSL2561_INTEGRATIONTIME_101MS);  // medium integration time (medium light)
    //_TSL2561.setTiming(TSL2561_INTEGRATIONTIME_402MS);  // longest integration time (dim light)

    // set the init var to true
    init_worked[TSL2561_ID] = true;
    return true;
  } else if(num == BME280_ID) {

    // start the communication with the BME280
    if(_BME280.begin() < 0) {
      #ifdef debug
        serial.println("void init_sensor(int num) - BME280 not inited!!");
      #endif
      return false;
    } else {
      #ifdef debug
        serial.println("void init_sensor(int num) - BME280 inited");
      #endif  

      init_worked[BME280_ID] = true;
      
      return true;
    }
  }
   /*else if(num == BQ34Z100_ID) {
    // according to the function the parameters are:
    //Battery Chemistry -- Number of series Batteries -- Battery capacity in mAh -- Current voltage on the pack in mV (eg 12.369V) -- Current being applied to the pack for currentShunt Cal in mA (must be > 200mA)
    _BQ34Z100.setup(0x101, BQ34Z100_SERIESCELLS, BQ34Z100_CELLCAPACITY, BQ34Z100_PACKVOLTAGE, BQ34Z100_APPLIEDCURRENT);
    delay(200);

    #ifdef debug
      serial.println("void init_sensor(int num) - can't check BQ34Z100 if it is inited...");
      serial.println("void init_sensor(int num) - continuing as inited...");
    #endif

    init_worked[BQ34Z100_ID] = false;
    return false;
 //   init_worked[BQ34Z100_ID] = true;
  
   // return true;
  } */
  
  else if(num == ANEMOMETER_ID) {

    pinMode(_ANEMOMETER_PIN, INPUT);
    analogReadResolution(12);
    
    #ifdef debug
      serial.println("void init_sensor(int num) - anemometer inited (pin to input)");
    #endif

    init_worked[ANEMOMETER_ID] = true;

    return true;
      
  } else if(num == RAIN_ID) {
    
    pinMode(_RAIN_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(_RAIN_PIN), ISR_RAIN, FALLING);

    #ifdef debug
      serial.println("void init_sensor(int num) - rain sensor inited");
    #endif

    init_worked[RAIN_ID] = true;

    return true;
    
  } else if(num == CO2_ID) {
    
    CO2_serial.begin(CO2_serial_baud);

    #ifdef debug
      serial.println("void init_sensor(int num) - CO2 inited");
    #endif

    init_worked[CO2_ID] = true;

    return true;
    
  } else if(num == TDR_ID) {

    TDR_serial.begin(TDR_serial_baud);

    #ifdef debug
      serial.println("void init_sensor(int num) - TDR inited");
    #endif

    init_worked[TDR_ID] = true;

    return true;
    
  }
  
  else {
    
    // we have not configured this ID of sensor
    #ifdef debug
      serial.println("void init_sensor(int num) - that number (ID) of a sensor doesn't excist");
    #endif
    return false;
    
  }
}

/*
 *  Function:    float get_sensor_data(int num)
 *  Parameter:   int num - the number of the sensor
 *  Description: return data from the sensor
 */
void get_sensor_data(int num) {

  #ifdef debug
    serial.print("int get_sensor_data(int num) - Selecting sensor number: "); serial.print(num); serial.println();
  #endif

  // inited = true
  if(init_worked[num] == true) {

    // L0 SENSOR 
    if(num == L0_ID) {
        float l0_data[L0_VAR_NUM] = {
          STM32L0.getTemperature(),              // get the temperature value
          STM32L0.getVDDA()                       // get the vdda value
        };
        
        #ifdef debug
          serial.print("int get_sensor_data(int num) - STM32L0 temp: "); serial.println(l0_data[0]); 
          serial.print("int get_sensor_data(int num) - STM32L0 VDD: ");  serial.println(l0_data[1]); 
        #endif

        l0_data[0] = int(l0_data[0] * 100);   
        l0_data[1] = int(l0_data[1] * 100);

        shifted_first  = data_index_row[num];            // as we are sending float value we always have to shift one place
        shifted_second = data_index_row[num] + 1;       // as we are sending float the second value (decimal) is shifted +1

        /*
         * 
         *  ARRAY OF 8BITS fits 4x2 DATA
         *  [x1][y1][x2][y2][x3][y3][x4][y4]
         * 
         *  shifted_first is always the X variable and shifted_second is shifted to the right (+1) which is the Y
         *  As we are going through the row, we increment the row number to get the second (Y) variable
         *  
         */

        /*
         *  int((stm32_temp - int(stm32_temp)) * 100) is not the full precision but good enough for the example
         *  
         *  Example:
         *  int((27.326 - 27) * 100) = int(0.326 * 100) = int(32.6) = 33
         * 
         * 
         */

        for(int num_counter=0; num_counter < L0_VAR_NUM; num_counter++) {
          data[num][num_counter][data_index_coloumn[num]][data_index_row[num] + shifted_first] = lowByte(int(l0_data[0] * 100));          // first element
          data[num][num_counter][data_index_coloumn[num]][data_index_row[num] + shifted_second] = highByte(int(l0_data[1] * 100));        // second element (decimal)
        }

        // go onto the next row
        data_index_row[num]++;

        // if we are out of index (more than 3 because we can only fit 4 data points (2x4))
        // !! for 8 data points 3->7
        if(data_index_row[num] > 3) {
          data_index_row[num] = 0;        // go back to null
          data_index_coloumn[num]++;      // go to the next coloumn
          
          // if we are overflowing
          if(data_index_coloumn[num] > 7) {
            data_index_coloumn[num] = 0;  // go back 
            data_coloumn_max[num] = 1;  
          } 
        }
    } else if(num == TSL2561_ID) {

      uint16_t tsl_data[TSL2561_VAR_NUM] = {
        _TSL2561.getLuminosity(TSL2561_VISIBLE),             // visible variable
        _TSL2561.getLuminosity(TSL2561_FULLSPECTRUM),        // full spectrum
        _TSL2561.getLuminosity(TSL2561_INFRARED)             // get infrared
       };

      #ifdef debug
        serial.print("int get_sensor_data(int num) - VISIBLE:  ");serial.println(tsl_data[0]);
        serial.print("int get_sensor_data(int num) - FULLSPEC: ");serial.println(tsl_data[1]);
        serial.print("int get_sensor_data(int num) - INFRARED: ");serial.println(tsl_data[2]);
      #endif    

      shifted_first  = data_index_row[num];            // as we are sending float value we always have to shift one place
      shifted_second = data_index_row[num] + 1;        // as we are sending float the second value (decimal) is shifted +1

      for(int num_counter = 0; num_counter < TSL2561_VAR_NUM; num_counter++) {
        data[num][num_counter][data_index_coloumn[num]][data_index_row[num] + shifted_first] = lowByte(tsl_data[num_counter]);             // first element
        data[num][num_counter][data_index_coloumn[num]][data_index_row[num] + shifted_second] = highByte(tsl_data[num_counter]);                   // second element (decimal)
      }

      // go onto the next row
      data_index_row[num]++;

      // if we are out of index (more than 3 because we can only fit 4 data points (2x4))
      // !! for 8 data points 3->7
      if(data_index_row[num] > 3) {
        data_index_row[num] = 0;        // go back to null
        data_index_coloumn[num]++;      // go to the next coloumn
          
        // if we are overflowing
        if(data_index_coloumn[num] > 7) {
          data_index_coloumn[num] = 0;  // go back 
          data_coloumn_max[num] = 1;    // we have overflown the coloumn
        } 
      }
    } else if(num == BME280_ID) {

      // get bme280 sensor data
      _BME280.readSensor();

      int bme_pressure_id     = 0;                                // id of bme pressure
      float bme_pressure      = _BME280.getPressure_Pa();         // data of bme pressure
      
      int bme_temperature_id  = 1;                                // id of bme temperature
      float bme_temperature   = _BME280.getTemperature_C();       // data of bme temperature

      int bme_humidity_id     = 2;                                // id of bme humidity
      float bme_humidity      = _BME280.getHumidity_RH();         // data of bme humidity

      #ifdef debug
        serial.print("int get_sensor_data(int num) - PRESSURE:  ");serial.println(bme_pressure,8);
        serial.print("int get_sensor_data(int num) - TEMPERATURE: ");serial.println(bme_temperature);
        serial.print("int get_sensor_data(int num) - HUMIDITY: ");serial.println(bme_humidity);
      #endif

      int int_bme_pressure      = int(bme_pressure * 100);
      int int_bme_temperature   = int(bme_temperature * 100);
      int int_bme_humidity      = int(bme_humidity * 100);

      shifted_first  = data_index_row[num];            // as we are sending float value we always have to shift one place
      shifted_second = data_index_row[num] + 1;        // as we are sending float the second value (decimal) is shifted +1

      // setting up the data
      data[num][bme_pressure_id][data_index_coloumn[num]][data_index_row[num] + shifted_first]        = lowByte(int_bme_pressure);           // shifted first set the one_pressure
      data[num][bme_pressure_id][data_index_coloumn[num]][data_index_row[num] + shifted_second]       = highByte(int_bme_pressure);          // shifted second set the two_pressure

      // temperature
      data[num][bme_temperature_id][data_index_coloumn[num]][data_index_row[num] + shifted_first]     = lowByte(int_bme_temperature);      // first element
      data[num][bme_temperature_id][data_index_coloumn[num]][data_index_row[num] + shifted_second]    = highByte(int_bme_temperature);    // second element (decimal)
      
      // humidity
      data[num][bme_humidity_id][data_index_coloumn[num]][data_index_row[num] + shifted_first]        = lowByte(int_bme_humidity);            // first element
      data[num][bme_humidity_id][data_index_coloumn[num]][data_index_row[num] + shifted_second]       = highByte(int_bme_humidity);          // second element (decimal)

      // go onto the next row
      data_index_row[num]++;

      // if we are out of index (more than 3 because we can only fit 4 data points (2x4))
      // !! for 8 data points 1->7
      if(data_index_row[num] > 3) {
        data_index_row[num] = 0;        // go back to null
        data_index_coloumn[num]++;      // go to the next coloumn
          
        // if we are overflowing
        if(data_index_coloumn[num] > 7) {
          data_index_coloumn[num] = 0;  // go back 
          data_coloumn_max[num] = 1;    // we have overflown the coloumn
        } 
      }
    } /*else if(num == BQ34Z100_ID) {

       // data of bq (/10 because of return of in x10 format)
       int bq_data[BQ34Z100_VAR_NUM] = {
        int(_BQ34Z100.getCurrent() / 10),       // current
        int(_BQ34Z100.getVoltage() / 10),       // voltage
        int(_BQ34Z100.getCapacity() / 10),      // capacity
        int(_BQ34Z100.getStatus())              // status
       };

       #ifdef debug
        serial.print("int get_sensor_data(int num) - CURRENT:  ");  serial.println(bq_data[0]);
        serial.print("int get_sensor_data(int num) - VOLTAGE: ");   serial.println(bq_data[1]);
        serial.print("int get_sensor_data(int num) - CAPACITY: ");  serial.println(bq_data[2]);
        serial.print("int get_sensor_data(int num) - STATUS: ");    serial.println(bq_data[3]);
       #endif

       shifted_first  = data_index_row[num];            // as we are sending float value we always have to shift one place
       shifted_second = data_index_row[num] + 1;        // as we are sending float the second value (decimal) is shifted +1

       // to save memory we loop through the data and set it to memory
       // we can use this because we are dividing all elements into first and second element
       for(int num_counter=0; num_counter < BQ34Z100_VAR_NUM; num_counter++) {
        data[num][num_counter][data_index_coloumn[num]][data_index_row[num] + shifted_first] = int(bq_data[num_counter]);                                  // first element
        data[num][num_counter][data_index_coloumn[num]][data_index_row[num] + shifted_second] = int((bq_data[num_counter] - int(bq_data[num_counter])) * 100);       // second element (decimal)
       }

       // go onto the next row
       data_index_row[num]++;

       // if we are out of index (more than 3 because we can only fit 4 data points (2x4))
       // !! for 8 data points 3->7
       if(data_index_row[num] > 3) {
        data_index_row[num] = 0;        // go back to null
        data_index_coloumn[num]++;      // go to the next coloumn
          
        // if we are overflowing
        if(data_index_coloumn[num] > 7) {
          data_index_coloumn[num] = 0;  // go back 
          data_coloumn_max[num] = 1;  
        } 
       }

    } */else if(num == ANEMOMETER_ID) {
       // we are reading range of 0,4V (0m/s) -> 2V(32.4m/s)
       // on 12bit we are getting 0-4094 value
       // 0,4V(496) -> 2V (2481)

       int anemometer_id      = 0;   
       int anemometer_data    = analogRead(_ANEMOMETER_PIN);

       // converting from 496-2482 to 0 - 32.4[m/s]
       float anemometer_data_converter = mapf(anemometer_data, 496,2482,0,32.4);

       int int_anemometar_data_converter = int(anemometer_data_converter * 100);

       #ifdef debug
        serial.print("int get_sensor_data(int num) - WIND[m/s]:  ");  serial.println(anemometer_data_converter);
       #endif

       shifted_first  = data_index_row[num];            // as we are sending float value we always have to shift one place
       shifted_second = data_index_row[num] + 1;        // as we are sending float the second value (decimal) is shifted +1

       data[num][anemometer_id][data_index_coloumn[num]][data_index_row[num] + shifted_first]   = lowByte(int_anemometar_data_converter);                                       // first element
       data[num][anemometer_id][data_index_coloumn[num]][data_index_row[num] + shifted_second]  = highByte(int_anemometar_data_converter);        // second element (decimal)

       // go onto the next row
       data_index_row[num]++;

       // if we are out of index (more than 3 because we can only fit 4 data points (2x4))
       // !! for 8 data points 3->7
       if(data_index_row[num] > 3) {
          data_index_row[num] = 0;        // go back to null
          data_index_coloumn[num]++;      // go to the next coloumn
            
          // if we are overflowing
          if(data_index_coloumn[num] > 7) {
            data_index_coloumn[num] = 0;  // go back 
            data_coloumn_max[num] = 1;  
          } 
       }
    }
    else if(num == RAIN_ID) {

      // get the total rainfall
      rain_totalRainfall = rain_tipCount * RAIN_BUCKET_SIZE;
      
      #ifdef debug
        serial.print("int get_sensor_data(int num) - RAIN tip count: ");serial.println(rain_tipCount);
        serial.print("int get_sensor_data(int num) - RAIN total rain fall"); serial.println(rain_totalRainfall);
      #endif

      data[num][0][data_index_coloumn[num]][data_index_row[num]] = int(rain_tipCount);                                // just send tip count becuase it can be calculated easily

      // next row
      data_index_row[num]++;

      // out of index for the row
      if(data_index_row[num] > 7) {

        data_index_row[num] = 0;                      // reset the row
        data_index_coloumn[num]++;                    // increment coloumn

        // if coloumn out of index
        if(data_index_coloumn[num] > 7) {
          data_index_coloumn[num] = 0;                // reset coloumn
          data_coloumn_max[num] = 1;                  // maximum reached
        }
        
      }
    } else if(num == CO2_ID) {

      String inString = "";
      int co2_value = 0;
        
      while(CO2_serial.available() > 0) {
        int inChar = CO2_serial.read();

        #ifdef debug
          serial.print("int get_sensor_data(int num) - ");serial.write(inChar);
        #endif
  
        if(inChar == 'z' || inChar == 'Z') {
          inString = "";
        }

        if(isDigit(inChar)) {
           inString += (char)inChar;
        }

        if(inChar == '\n') {
          co2_value = (inString.toInt() / 10);

         
        }
        
      }

      #ifdef debug
        serial.print("int get_sensor_data(int num) - ppm:");serial.println(co2_value);
      #endif

      shifted_first  = data_index_row[num];            // as we are sending float value we always have to shift one place
      shifted_second = data_index_row[num] + 1;        // as we are sending float the second value (decimal) is shifted +1

      data[num][0][data_index_coloumn[num]][data_index_row[num] + shifted_first] = lowByte(co2_value);
      data[num][0][data_index_coloumn[num]][data_index_row[num] + shifted_second] = highByte(co2_value);
      // WARNING!
      // on the master side the value you get multiple by x100!!!!

       // next row
      data_index_row[num]++;

      // out of index for the row
      if(data_index_row[num] > 7) {

        data_index_row[num] = 0;                      // reset the row
        data_index_coloumn[num]++;                    // increment coloumn

        // if coloumn out of index
        if(data_index_coloumn[num] > 7) {
          data_index_coloumn[num] = 0;                // reset coloumn
          data_coloumn_max[num] = 1;                  // maximum reached
        }
        
      }
      
    } else if(num == TDR_ID) {
      
      String inString = "";
      int TDR_counter = 0;

      int TDR_data[TDR_VAR_NUM] = {
          0,      // TDR_vol_w_content
          0,      // TDR_soil_temp
          0,      // TDR_soil_perm
          0       // TDR_soil_elec
      };

      while(TDR_serial.available() > 0) {
        int inChar = TDR_serial.read();

        #ifdef debug
          serial.print("int get_sensor_data(int num) - ");serial.write(inChar);
        #endif

        if(inChar == '5' && TDR_counter == 0) {
           // the first 5
        } else {

          if(inChar == '+') {
            switch(TDR_counter) {
              case 1: 
                TDR_data[TDR_counter - 1] = inString.toInt();
                #ifdef debug
                  serial.print("int get_sensor_data(int num) - vol_w_content:"); serial.println(TDR_data[TDR_counter - 1]);
                #endif
                break;
              case 2: 
                TDR_data[TDR_counter - 1] = inString.toInt();
                #ifdef debug
                  serial.print("int get_sensor_data(int num) - soil_temp:"); serial.println(TDR_data[TDR_counter - 1]);
                #endif
                break;
              case 3:
                TDR_data[TDR_counter - 1] = inString.toInt();
                #ifdef debug
                  serial.print("int get_sensor_data(int num) - soil_perm:"); serial.println(TDR_data[TDR_counter - 1]);
                #endif
                break;
              case 4:
                TDR_data[TDR_counter - 1] = inString.toInt();
                #ifdef debug
                  serial.print("int get_sensor_data(int num) - soil_elec:"); serial.println(TDR_data[TDR_counter - 1]);
                #endif
                break;
            }
            
            TDR_counter++;
            inString = "";
          } else if(inChar == '\r') {
            //just ignore 
          } else if(inChar == '\n') {
            //end of everything just escape the while loop
            break;
          }
          else {
            if(isDigit(inChar)) {
             inString += (char)inChar;
            }
          }
        }
      }// end of while

      shifted_first  = data_index_row[num];            // as we are sending float value we always have to shift one place
      shifted_second = data_index_row[num] + 1;        // as we are sending float the second value (decimal) is shifted +1

      for(int TDR_id_counter = 0; TDR_id_counter < TDR_VAR_NUM; TDR_id_counter++) {

        data[num][TDR_id_counter][data_index_coloumn[num]][data_index_row[num] + shifted_first]   = lowByte(TDR_data[TDR_id_counter]);         // first element
        data[num][TDR_id_counter][data_index_coloumn[num]][data_index_row[num] + shifted_second]  = highByte(TDR_data[TDR_id_counter]);        // second element (decimal)

      }
      // go onto the next row
      data_index_row[num]++;

      // if we are out of index (more than 3 because we can only fit 4 data points (2x4))
      // !! for 8 data points 3->7
      if(data_index_row[num] > 3) {
        data_index_row[num] = 0;        // go back to null
        data_index_coloumn[num]++;      // go to the next coloumn
            
        // if we are overflowing
        if(data_index_coloumn[num] > 7) {
          data_index_coloumn[num] = 0;  // go back 
          data_coloumn_max[num] = 1;  
        } 
     }
      
    }
    else {
      #ifdef debug
        serial.println("int get_sensor_data(int num) - that number (ID) of a sensor doesn't excist");
      #endif
    }

    // print data array
    print_data();
    
  } else {
    #ifdef debug
      serial.println("int get_sensor_data(int num) - the sensor did not init in the init section");
    #endif
  }
}

/*
 *  Function:    byte id_by_resistors()
 *  Paramter:    NULL
 *  Description: set the ID that we will use on the CAN BUS by soldering the resistors (not implemented into hardware yet)
 */
byte id_by_resistors() {

  // variables
  byte address = 0;
  int _ID0, _ID1, _ID2;

  // setup pins
  pinMode(ID0_PIN, INPUT); 
  pinMode(ID1_PIN, INPUT);
  pinMode(ID2_PIN, INPUT);

  // read pins
  _ID0 = digitalRead(ID0_PIN); _ID1 = digitalRead(ID1_PIN); _ID2 = digitalRead(ID2_PIN);

  #ifdef debug
    serial.println("byte id_by_resistors() - ID pin status");
    serial.println("byte id_by_resistors() - ID0: ");serial.print(_ID0);
    serial.println("byte id_by_resistors() - ID1: ");serial.print(_ID1);
    serial.println("byte id_by_resistors() - ID2: ");serial.print(_ID2);
    serial.println();
  #endif

  bitWrite(address, 0, _ID0);     // writing into the 0th position of the byte
  bitWrite(address, 1, _ID1);     // writing into the 1st position of the byte
  bitWrite(address, 2, _ID2);     // writing into the 2nd position of the byte

  #ifdef debug
    serial.println("byte id_by_resistors() - Address of the ID will be: ");
    serial.print(address);
    serial.println();
  #endif
  
  return address;
}

/*
 *  Function:     void CAN_setup()
 *  Description:  setup the CAN bus
 */
void CAN_setup() {
  //CAN_BUS
  if(CAN_BUS.begin(MCP_ANY, CAN_SPEED, CAN_MHZ) == CAN_OK &&
     CAN_BUS.setMode(MCP_NORMAL) == MCP2515_OK ){

    // enable wakeup on interrupt - for MCP
    CAN_BUS.setSleepWakeup(1);

    // can interrupt pins setup
    pinMode(CAN_PIN_INT, INPUT);
    attachInterrupt(digitalPinToInterrupt(CAN_PIN_INT), ISR_CAN, FALLING); 
    
    #ifdef debug
      serial.println("void CAN_setup() - CAN bus all inited, ready to go!");
    #endif
    
  } else {
    // CAN didn't init
    #ifdef debug
      serial.println("void CAN_setup() - CAN bus failed to init... halting system");
    #endif
    while(1);    
  }
}

/*
 *  Function:     void sleep_devices(void) 
 *  Description:  send devices to sleep
 */
void sleep_devices(void) {

  #ifdef debug
    serial.println("void sleep_devices(void) - going into sleep");
  #endif
  
  CAN_BUS.setMode(MCP_SLEEP);
  // STM32L0
  STM32L0.stop(TIMER_SECOND*1000);
  //research: https://github.com/GrumpyOldPizza/ArduinoCore-stm32l0/blob/18fb1cc81c6bc91b25e3346595f820985f2267e5/system/STM32L0xx/Source/stm32l0_system.c wakeup manually
}

void setup() {
  
  // Setup serial
  serial.begin(SERIAL_SPEED);

  // Setup CAN bus
  CAN_setup();

  // for loop through all the sensors and init them
  for(int sensor=0; sensor < number_of_sensors; sensor++) {
    init_sensor(sensor);
  }

}

void loop() {

  // if the can interrupt has been set
  if(send_via_int) {

    byte sndStat;                 // stat of sending CAN to master
    byte data_begin[2];           // array for pre-sending info
    int chosen_sensor = 0;        // var for for loop
    
    exec_int_can = false;         // execute to false to not have mutliple exec

    // choose the sensor
    switch(CAN_RXID) {
      case L0_CANID:
      
        #ifdef debug
          serial.println("void loop() - CAN_RXID is L0");
        #endif

        // L0 I choose you!
        chosen_sensor = L0_ID;

        // get the sensor data
        get_sensor_data(L0_ID);

        // number of variables we are using
        data_begin[1] = L0_VAR_NUM;

        
        break;
      case TSL2561_CANID:
      
        #ifdef debug
          serial.println("void loop() - CAN_RXID is TSL2561");
        #endif

        // TSL2561 I choose you!
        chosen_sensor = TSL2561_ID;

        // get the sensor data
        get_sensor_data(TSL2561_ID);

        // number of variables we are using
        data_begin[1] = TSL2561_VAR_NUM;

        break;
      /*case BQ34Z100_CANID:
      
        #ifdef debug
          serial.println("void loop() - CAN_RXID is BQ34Z100");
        #endif

        // BQ34Z100 I choose you!
        chosen_sensor = BQ34Z100_ID;

        // get sensor data
        get_sensor_data(BQ34Z100_ID);

        // number of variables we are using
        data_begin[1] = BQ34Z100_VAR_NUM;
        
        break;*/
      case ANEMOMETER_CANID:

        #ifdef debug
          serial.println("void loop() - CANRXID is ANEMOMETER");
        #endif
        
          //ANEMOMETER I choose you!
          chosen_sensor = ANEMOMETER_ID;

          // get sensor data
          get_sensor_data(ANEMOMETER_ID);

          // number of variables we are using
          data_begin[1] = ANEMOMETER_VAR_NUM;
        
        break;
      case RAIN_CANID:

        #ifdef debug
          serial.println("void loop() - CANRXID is RAIN");
        #endif

        // RAIN I choose you!
        chosen_sensor = RAIN_ID;

        // get the RAIN data
        get_sensor_data(RAIN_ID);

        // number of variables from rain
        data_begin[1] = RAIN_VAR_NUM;

        break;
      case CO2_CANID:

        #ifdef debug
          serial.println("void loop() - CANRXID is CO2");
        #endif

        chosen_sensor = CO2_ID;

        get_sensor_data(CO2_ID);

        data_begin[1] = CO2_VAR_NUM;
        break;
      
    }

    // check if we had a max value (8 coloumns filled)
    if(data_coloumn_max[chosen_sensor] == true) {

      // tell master that it should expect 8 coloumns (1+7)
      data_begin[0]= 7;

      // for the for loop below to send all 8
      data_index_coloumn[chosen_sensor] = 7;
          
    } else { 
      
      // just paste how many are there 
      data_begin[0] = data_index_coloumn[chosen_sensor]; 
      
    }
    
    // send coloumn data
    sndStat = CAN_BUS.sendMsgBuf(CAN_MASTER_ID, 0, 2, data_begin); if(sndStat == CAN_OK) { 
      #ifdef debug 
        serial.println("void loop() - Message Sent Successfully!"); 
      #endif
    } else { 
      #ifdef debug
        serial.println("void loop() - Error Sending Message..."); 
      #endif  
    }

    // loop through the variables 
    for(int varz=0; varz<data_begin[1];varz++) {
      // loop through the coloumns
      for(int i=0; i < data_index_coloumn[chosen_sensor]+1; i++) {
        // if it is not a full row (not 8bits) send as many as avaible (data_index_row)
        if(i == data_index_coloumn[chosen_sensor] && data_coloumn_max[chosen_sensor] == 0) {
          sndStat = CAN_BUS.sendMsgBuf(L0_CANID, 0, data_index_row[chosen_sensor], data[chosen_sensor][varz][i]); if(sndStat == CAN_OK) { 
            #ifdef debug
              serial.println("void loop() - Message Sent Successfully!");
            #endif
          } else { 
            #ifdef debug
              serial.println("void loop() - Error Sending Message..."); 
            #endif
          }
        }else {
          // 8bits available, send all 8
          sndStat = CAN_BUS.sendMsgBuf(L0_CANID, 0, 8,  data[chosen_sensor][varz][i]); if(sndStat == CAN_OK) { 
            #ifdef debug
              serial.println("void loop() - Message Sent Successfully!"); 
            #endif
          } else { 
            #ifdef debug
              serial.println("void loop() - Error Sending Message..."); 
            #endif
          }
        }
      }
    }

    // set all coloumn and row values to 0
    for(int i=0; i<number_of_sensors; i++) {
      data_index_row[i]     = 0;                    // reset row
      data_index_coloumn[i] = 0;                    // reset coloumn
      data_coloumn_max[i]   = false;                // reset max coloumn number to 0
    }

    // clear every data
    memset(data, 0, sizeof(data));
    
    send_via_int = false;         // clear the interrupt flag
    exec_int_can = true;          // allow interrupt to be exec
    
  } else {
    // not_my_id must be false because we only execute when we read our ID
    // send_via_int must be false so that we don't execute two times
    if(not_my_id == false && send_via_int == false) {
      sleep_devices();
      
      if(not_my_id == false && send_via_int == false) {
        // loop through the sensors and get data
        for(int sensor=0; sensor < number_of_sensors; sensor++) {
          get_sensor_data(sensor);
        }
        
        not_my_id = false;
      }
      
      not_my_id = false;
    }
  }  
}

/*
 *  Function:    void ISR_CAN()
 *  Description: interrupt function for CAN bus
 */
void ISR_CAN()
{
  // wakeup the stm32l0
  STM32L0.wakeup();
  
  // if we can execute via interrupt
  if(exec_int_can) {

    #ifdef debug
      serial.println("void ISR_CAN() - ISR executed");
    #endif

    // read the id and to clear the interrupt flag
    CAN_BUS.readMsgBuf(&CAN_RXID,NULL,NULL);
    
    #ifdef debug
      serial.print("ID: 0x");
      serial.println(CAN_RXID, HEX);
    #endif

    // if we are in the id spectrum (if our ID is 0x101 then 0x101 will pass, 0x201 will not )
    if((CAN_ID & CAN_RXID) == int(CAN_ID)) {
      send_via_int = true;                                            // we can send by interrupt
      not_my_id = false;                                              // it is our id

    } else {
      #ifdef debug
        serial.println("ISR_CAN() - not my ID");
      #endif
      send_via_int = false;                                           // do not send!
      not_my_id = true;                                               // not our ID
      CAN_RXID = 0;
    }

  } else {
    serial.println("void ISR_CAN() - received but not exec");
  }
}

/*
 *  Function:     void ISR_RAIN()
 *  Description:  Interrupt on rain drop
 */
void ISR_RAIN() {

  // wakeup the STM32L0
  STM32L0.wakeup();

  #ifdef debug
    serial.println("void ISR_RAIN() - interrupt on rain drop");
  #endif

  // a small debounce
  if((millis() - rain_ContactTime) > 70) {
    rain_tipCount++;                                              // increment the rain drop
    rain_ContactTime = millis();                                  // for debounce
  }

  // dont reboot just ignore everything
  send_via_int = false;                                           // do not send!
  not_my_id = true;                                               // not our ID
  CAN_RXID = 0;
}

