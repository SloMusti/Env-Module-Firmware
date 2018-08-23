/*
 * 
 * 1. INIT
 * 2. SLEEP
 * 
 * 3.1 INTERRUPT FROM CAN
 *      -> WAKE UP
 *      -> REPORT VALUES TO MASTER
 *      -> 2.
 * 3.2 INTERRUPT FROM TIMER (60s)
 *      -> WAKE UP
 *      -> REPORT VALUES TO MASTER
 *      -> 2.
 * 
 */
// SENSOR DEFINES
#define L0_TEMP 0 // STM32L0 internal temperature reading
 
// EXTRA DEFINES
#define debug                 // if defined debug will show on serial       
#define TIMER_SECOND 10

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

#define CAN_PIN_INT 4         // interrupt pin PB5
#define CAN_PIN_NSS 9         // chip-select PB12-NSS pin

#define CAN_SPEED CAN_500KBPS // CAN_125KBPS
#define CAN_MHZ   MCP_8MHZ    // MCP_16MHZ, MCP_20MHZ

MCP_CAN CAN_BUS(CAN_PIN_NSS); // can bus object

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
char msgString[128];                        // Array to store serial string


// SENSORS


// EXTRA LINKS
// https://github.com/GrumpyOldPizza/ArduinoCore-stm32l0/blob/18fb1cc81c6bc91b25e3346595f820985f2267e5/system/STM32L0xx/Source/stm32l0_system.c wakeup manually

bool can_int = false;

byte data[1] = {0x00};

/*
 *  Function:    void init_sensor(int num)
 *  Parameter:   int num - the number of the sensor
 *  Description: Choose your sensor by `num` and your sensor gets inited
 */
bool init_sensor(int num) {

  #ifdef debug
    serial.print("void select_sensor(int num) - Selecting sensor number: "); serial.print(num); serial.println();
  #endif
  switch(num) {
    case L0_TEMP:
      // nothing to init
      serial.println("void select_sensor(int num) - L0_TEMP inited");
      return true;
      break;
    default:
      #ifdef debug
        serial.println("void select_sensor(int num) - No sensor detected for init");
      #endif 
      return false;
      break;
  }
}

/*
 *  Function:    float get_sensor_data(int num)
 *  Parameter:   int num - the number of the sensor
 *  Description: return data from the sensor
 */
float get_sensor_data(int num) {
  
  #ifdef debug
    serial.print("int get_sensor_data(int num) - Selecting sensor number: "); serial.print(num); serial.println();
  #endif
  switch(num) {
    case 0:
      float stm32_temp = STM32L0.getTemperature();
      #ifdef debug
        serial.print("int get_sensor_data(int num) - STM32L0 temp: "); serial.print(stm32_temp); serial.println();
      #endif

      data[0] = int(stm32_temp);
      
      return stm32_temp;
      break;
    /*default:
      #ifdef debug
        serial.println("int get_sensor_data(int num) - No sensor detected for init");
      #endif 
      return 0;*/
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

  serial.println("void sleep_devices(void) - going into sleep");
  // STM32L0
  STM32L0.stop(TIMER_SECOND*1000);
  //research: https://github.com/GrumpyOldPizza/ArduinoCore-stm32l0/blob/18fb1cc81c6bc91b25e3346595f820985f2267e5/system/STM32L0xx/Source/stm32l0_system.c wakeup manually
}


void setup() {

  // Setup serial
  serial.begin(SERIAL_SPEED);

  // Setup CAN bus
  CAN_setup();

  init_sensor(L0_TEMP);

}

void loop() {

  sleep_devices();
  get_sensor_data(L0_TEMP);
  
}
void ISR_CAN()
{
    serial.println("void ISR_CAN() - ISR executed");
    serial.println(CAN_BUS.readMsgBuf(NULL,NULL,NULL));
    
    byte sndStat = CAN_BUS.sendMsgBuf(0x100, 0, 1, data);
    if(sndStat == CAN_OK){
      serial.println("void ISR_CAN() - Message Sent Successfully!");
    } else {
      serial.println("void ISR_CAN() - Error Sending Message...");
    }
  
}
