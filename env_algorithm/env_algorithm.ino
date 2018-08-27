// SENSOR DEFINES
#define L0_TEMP 0 // STM32L0 internal temperature reading
#define L0_VDD  1 // STM32L0 internal VCC ADC
 
// EXTRA DEFINES
#define debug                 // if defined debug will show on serial       
#define TIMER_SECOND 60        // every TIMER_SECOND it wakes up

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

#define CAN_PIN_INT 4             // interrupt pin PB5
#define CAN_PIN_NSS 9             // chip-select PB12-NSS pin

#define CAN_ID 0x100

#define CAN_SPEED CAN_500KBPS     // CAN_125KBPS
#define CAN_MHZ   MCP_8MHZ        // MCP_16MHZ, MCP_20MHZ

MCP_CAN CAN_BUS(CAN_PIN_NSS);     // can bus object

bool send_via_int = false;        // flag to send via the interrupt
bool exec_int_can = true;         // execute the interrupt can
bool not_my_id = false;       

const int number_of_sensors = 2;        // how many sensors are there

// index coloumn                        index row 
int data_index_coloumn[number_of_sensors];
int data_index_row[number_of_sensors];   

// SENSORS


// EXTRA LINKS
// https://github.com/GrumpyOldPizza/ArduinoCore-stm32l0/blob/18fb1cc81c6bc91b25e3346595f820985f2267e5/system/STM32L0xx/Source/stm32l0_system.c wakeup manually

// store sensor data
// data_x[      sensors    ][vars][x*8bits][8 bits]
byte data[number_of_sensors][  4 ][   8   ][  8   ];  // number of sensors * variables * 64 bit
bool init_worked[number_of_sensors];                  // check if it has inited correctly 

/*
 *  Function:    void init_sensor(int num)
 *  Parameter:   int num - the number of the sensor
 *  Description: Choose your sensor by `num` and your sensor gets inited
 */
bool init_sensor(int num) {

  #ifdef debug
    serial.print("void select_sensor(int num) - Selecting sensor number: "); serial.print(num); serial.println();
  #endif
                      
  if(num == L0_TEMP) {
    #ifdef debug
      serial.println("void select_sensor(int num) - L0_TEMP inited");
    #endif 
    
    init_worked[L0_TEMP] = true;
    return true;
  } else if(num == L0_VDD) {
    #ifdef debug
      serial.println("void select_sensor(int num) - L0_VDD inited");
    #endif

    init_worked[L0_VDD] = true;
    return true;
  } else {
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
    if(num == L0_TEMP) {
      
        int stm32_temp = int(STM32L0.getTemperature());
        
        #ifdef debug
          serial.print("int get_sensor_data(int num) - STM32L0 temp: "); serial.print(stm32_temp); serial.println();
        #endif
        
    } else if(num == L0_VDD) {
      
        float stm32_vdd = STM32L0.getVDDA();
        
        #ifdef debug
          serial.print("int get_sensor_data(int num) - STM32L0 VDD: "); serial.print(stm32_vdd); serial.println();
        #endif
        
    } else {
      #ifdef debug
        serial.println("int get_sensor_data(int num) - that number (ID) of a sensor doesn't excist");
      #endif
    }
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
    
    #ifdef debug
      serial.println("void CAN_setup() - CAN bus failed to init...");
      while(1);
    #endif
    
  }
}

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

  for(int sensor=0; sensor < number_of_sensors; sensor++) {
    init_sensor(sensor);
  }

}

void loop() {

  // if the can interrupt has been set
  if(send_via_int) {
    int num_end = 0;
    
    exec_int_can = false;         // execute to false to not have mutliple exec

    // loop through the sensors and get data 
    for(int sensor=0; sensor < number_of_sensors; sensor++) {
      get_sensor_data(sensor);
    }

    // data_begin so that the master knows how many coloumns to expect
 /*  byte data_begin[1]; data_begin[0] = data_index_coloumn;
    byte sndStat;

    // send coloumn data
    sndStat = CAN_BUS.sendMsgBuf(0x100, 0, 1, data_begin); if(sndStat == CAN_OK){ serial.println("void loop() - Message Sent Successfully!"); } else { serial.println("void loop() - Error Sending Message..."); }

    // loop through the coloumns
    for(int i=0; i < data_index_coloumn+1; i++) {
      // if it is not a full row (not 8bits) send as many as avaible (data_index_row)
      if(i == data_index_coloumn) {
        sndStat = CAN_BUS.sendMsgBuf(0x100, 0, data_index_row, data[i]); if(sndStat == CAN_OK){ serial.println("void loop() - Message Sent Successfully!"); } else { serial.println("void loop() - Error Sending Message..."); }
      }else {
        // 8bits available send all 8
        sndStat = CAN_BUS.sendMsgBuf(0x100, 0, 8, data[i]); if(sndStat == CAN_OK){ serial.println("void loop() - Message Sent Successfully!"); } else { serial.println("void loop() - Error Sending Message..."); }
      }
    }

    // clear array
    data_index_coloumn = 0;       // reset coloumn
    data_index_row = 0;           // reset row*/

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

// used by ISR_CAN
long unsigned int rxId;

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
    CAN_BUS.readMsgBuf(&rxId,NULL,NULL);
    
    #ifdef debug
      serial.print("ID: 0x");
      serial.println(rxId, HEX);
    #endif

    // if we are in the id spectrum (if our ID is 0x101 then 0x101 will pass, 0x201 will not )
    if((CAN_ID & rxId) == int(CAN_ID)) {
      send_via_int = true;                                            // we can send by interrupt
      not_my_id = false;                                              // it is our id
    } else {
      #ifdef debug
        serial.println("ISR_CAN() - not my ID");serial.println();
      #endif
      send_via_int = false;                                           // do not send!
      not_my_id = true;                                               // not our ID
    }

  } else {
    serial.println("void ISR_CAN() - received but not exec");
  }
  
}
