#include "can_module.h"
#include "global.h"

CAN_MODULE thisClass;

MCP_CAN CAN_BUS(CAN_PIN_NSS); // can bus object

int local_send_data_int = false;
long unsigned int CAN_RXID = 0;

bool CAN_MODULE::return_local_send_data_int() {
    return local_send_data_int;
}

void CAN_MODULE::set_local_send_data_int(bool status) {
    local_send_data_int = status;
}

long unsigned int CAN_MODULE::return_CAN_RXID() {
    return CAN_RXID;
}

void CAN_MODULE::set_CAN_RXID(long unsigned int ID) {
    CAN_RXID = ID;
}

/*
 *  Function: static void ISR()
 *  Description: it is a function that is a refrence to ISR_CAN()
 */
static void ISR() { 
    thisClass.ISR_CAN();
}

/*
 *  Function:       void ISR_CAN()
 *  Description:    Interrupt for CAN
 */
void CAN_MODULE::ISR_CAN() {

    // wakeup the STM32L0
    STM32L0.wakeup();

    if(execute_int_can) {
        
        #ifdef debug
            serial_debug.println("can_module (ISR_CAN) - ISR");
        #endif

        // read the id and to clear the interrupt flag
        CAN_BUS.readMsgBuf(&CAN_RXID,NULL,NULL);
    
        #ifdef debug
            serial_debug.print("can_module (ISR_CAN) - RXID: 0x");
            serial_debug.println(CAN_RXID, HEX);
        #endif



        if((CAN_ID & CAN_RXID) == int(CAN_ID)) {
            local_send_data_int   = true;
        } else {
            local_send_data_int   = false;
            CAN_RXID        = 0;

            #ifdef debug
                serial_debug.println("can_module (ISR_CAN) - not my ID!");
            #endif
        }
    } // end of if
} // end of ISR_CAN()

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
        attachInterrupt(digitalPinToInterrupt(CAN_PIN_INT), ISR, FALLING); 

        return true;
    } else {
        return false;
    }

} // end of setup()



/*
 *  Function:       bool CAN_MODULE::send_data(...)
 *  Description:    send the data
 */
bool CAN_MODULE::send_data( byte data[][8][8],
                            byte time_data[8][8],
                            int number_of_variables,
                            int col,
                            int row,
                            bool counter_col_overflow ) {

    byte send_data_status   = CAN_OK;       // data status
    byte data_begin[2];                     // the array we send first time

    // setup the begin data
    if(counter_col_overflow) {
        data_begin[0]       = 7;            // tell master that it should expect 8 coloumns
        col                 = 7;            // for the for loop
    } else {
        data_begin[0]       = col;          // tell master how many coloumns to expect
    }

    data_begin[1] = number_of_variables;    // send maste rhow many vairables there are * coloumn

    // send the begin data
    send_data_status = CAN_BUS.sendMsgBuf(CAN_MASTER_ID, 0, 2, data_begin);
    if(send_data_status != CAN_OK)
        return false;

    // SENDING THE DATA
    // loop through the variables - data[x][][]
    for(int variables=0; variables<data_begin[1]; variables++) {
        // loop through the coloumn - data[][x][]
        for(int coloumn=0; coloumn<col+1; coloumn++) {
            // not all 8bits are available, so just send as many as available
            if(coloumn == col && counter_col_overflow == false) {
                
                send_data_status = CAN_BUS.sendMsgBuf(CAN_MASTER_ID, 0, row, data[variables][coloumn]);
                if(send_data_status != CAN_OK) 
                    return false;

                send_data_status = CAN_BUS.sendMsgBuf(CAN_MASTER_ID, 0, row, time_data[coloumn]);
                if(send_data_status != CAN_OK) 
                    return false;

            } else {

                send_data_status = CAN_BUS.sendMsgBuf(CAN_MASTER_ID, 0, 8, data[variables][coloumn]);
                if(send_data_status != CAN_OK) 
                    return false;

                send_data_status = CAN_BUS.sendMsgBuf(CAN_MASTER_ID, 0, 8, time_data[coloumn]);
                if(send_data_status != CAN_OK) 
                    return false;
            
            }
        } // end of coloumn for loop
    } // end of variables for loop

    return true;

} // end of send_data()

/*
 *  Function:       int CAN_MODULE::set_sensor_CAN_id(int& ID, int num)
 *  Description:    set the sensors CAN id automatically based on num
 */
int CAN_MODULE::set_sensor_CAN_id(int& ID, int num) {

    // example: ID = 0x100 | 1 -> 0x101
    ID = CAN_ID | num;
    return ID;

} // end of set_sensor_CAN_id()

/*
 *  Function:       int CAN_MODULE::get_sensor_CAN_id(int num)
 *  Description:    return the sensor ID from the num
 */
int CAN_MODULE::get_sensor_CAN_id(int ID) {
   serial_debug.println(ID);
   serial_debug.println(ID | CAN_ID);
   return (ID | CAN_ID) - 256;
    
} // end of get_sensor_CAN_id()