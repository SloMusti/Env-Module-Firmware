// SENSOR DEFINES
#define L0 0                        // STM32L0 
#define L0_CANID 0x101              // the ID through CAN
#define L0_VAR_NUM 2

const int number_of_sensors = 1;    // how many sensors are there

 
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
#define serial       Serial1        // the serial we are using for debug
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

int data_index_coloumn[number_of_sensors];    // coloumn array
int data_index_row[number_of_sensors];        // row array
bool data_coloumn_max[number_of_sensors];     // max coloumn value 

// SENSORS

// store sensor data
// data_x[      sensors    ][vars][x*8bits][8 bits]
byte data[number_of_sensors][  4 ][   8   ][  8   ];  // number of sensors * variables * 64 bit
bool init_worked[number_of_sensors];                  // check if it has inited correctly 

/*
 *  Function: void print_data()
 *  Description: print data fom data array
 *  Parameter: x
 */
void print_data() {
  serial.println("************************************************");
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
  }
  serial.println("************************************************");
}

/*
 *  Function:    void init_sensor(int num)
 *  Parameter:   int num - the number of the sensor
 *  Description: Choose your sensor by `num` and your sensor gets inited
 */
bool init_sensor(int num) {

  /*#ifdef debug
    serial.print("void select_sensor(int num) - Selecting sensor number: "); serial.print(num); serial.println();
  #endif*/
                      
  if(num == L0) {
    #ifdef debug
      serial.println("void select_sensor(int num) - L0 MCU inited");
    #endif 

    // set the init var to true
    init_worked[L0] = true;
    return true;
  } else {
    
    // we have not configured this ID of sensor
    #ifdef debug
      serial.println("void select_sensor(int num) - that number (ID) of a sensor doesn't excist");
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
    if(num == L0) {
        
        int stm32_temp_variable_id  = 0;                        // so we know which variable we use
        float stm32_temp = STM32L0.getTemperature();            // get the value

        int stm32_vdd_variable_id   = 1;                        // VAR 1
        float stm32_vdd = STM32L0.getVDDA();                    // get the value
        
        #ifdef debug
          serial.print("int get_sensor_data(int num) - STM32L0 temp: "); serial.print(stm32_temp); serial.println();
          serial.print("int get_sensor_data(int num) - STM32L0 VDD: ");  serial.print(stm32_vdd); serial.println();
        #endif

        int shifted_first  = data_index_row[num];            // as we are sending float value we always have to shift one place
        int shifted_second = data_index_row[num] + 1;       // as we are sending float the second value (decimal) is shifted +1

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

        // setting the temperature variable
        data[num][stm32_temp_variable_id][data_index_coloumn[num]][data_index_row[num] + shifted_first] = int(stm32_temp);                              // first element
        data[num][stm32_temp_variable_id][data_index_coloumn[num]][data_index_row[num] + shifted_second] = int((stm32_temp - int(stm32_temp)) * 100);   // second element (decimal)
        
        // setting the vdd variable
        data[num][stm32_vdd_variable_id][data_index_coloumn[num]][data_index_row[num] + shifted_first] = int(stm32_vdd);
        data[num][stm32_vdd_variable_id][data_index_coloumn[num]][data_index_row[num] + shifted_second] = int((stm32_vdd - int(stm32_vdd)) * 100);

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
    } else {
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
        chosen_sensor = L0;

        // get the sensor data
        get_sensor_data(L0);

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

        // number of variables we are using
        data_begin[1] = L0_VAR_NUM;
        
        break;
    }
    
    // send coloumn data
    sndStat = CAN_BUS.sendMsgBuf(CAN_MASTER_ID, 0, 2, data_begin); if(sndStat == CAN_OK){ serial.println("void loop() - Message Sent Successfully!"); } else { serial.println("void loop() - Error Sending Message..."); }

    // loop through the variables 
    for(int varz=0; varz<L0_VAR_NUM;varz++) {
      // loop through the coloumns
      for(int i=0; i < data_index_coloumn[chosen_sensor]+1; i++) {
        // if it is not a full row (not 8bits) send as many as avaible (data_index_row)
        if(i == data_index_coloumn[chosen_sensor] && data_coloumn_max[chosen_sensor] == 0) {
          sndStat = CAN_BUS.sendMsgBuf(L0_CANID, 0, data_index_row[chosen_sensor], data[chosen_sensor][varz][i]); if(sndStat == CAN_OK){ serial.println("void loop() - Message Sent Successfully!"); } else { serial.println("void loop() - Error Sending Message..."); }
        }else {
          // 8bits available, send all 8
          sndStat = CAN_BUS.sendMsgBuf(L0_CANID, 0, 8,  data[chosen_sensor][varz][i]); if(sndStat == CAN_OK){ serial.println("void loop() - Message Sent Successfully!"); } else { serial.println("void loop() - Error Sending Message..."); }
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

      // add filtering here
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
