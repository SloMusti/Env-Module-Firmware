#include "can_module.h"

MCP_CAN CAN_BUS(CAN_PIN_NSS); // can bus object*/

/*
 *  Function:       void ISR_CAN()
 *  Description:    Interrupt for CAN
 */
void ISR_CAN() {

}

/*
 *  Function:       bool CAN_MODULE::setup()
 *  Description:    setup the CAN module
 */
bool CAN_MODULE::setup() {
    if( CAN_BUS.begin(MCP_ANY, CAN_SPEED, CAN_MHZ) == CAN_OK && 
        CAN_BUS.setMode(MCP_NORMAL) == MCP2515_OK ) {

        // set the sleep wakeup
        CAN_BUS.setSleepWakeup(1);

        // setup the interrupt pin
        pinMode(CAN_PIN_INT, INPUT);
        attachInterrupt(digitalPinToInterrupt(CAN_PIN_INT), ISR_CAN, FALLING); 

        return true;
    } else {
        return false;
    }
} // end of setup()

/*
 *  Function: void CAN_MODULE::set_sensor_CAN_id(int& ID, int num)
 *  Description: set the sensors CAN id automatically based on num
 */
void CAN_MODULE::set_sensor_CAN_id(int& ID, int num) {

    // example: ID = 0x100 | 1 -> 0x101
    ID = CAN_ID | num;

} // end of set_sensor_CAN_id()