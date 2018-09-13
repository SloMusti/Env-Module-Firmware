#include "TDR.h"
#include "global.h"

/*
 *  Function:       bool TDR::setup()
 *  Description:    setup the serial
 */
bool TDR::setup() {

    TDR_SERIAL.begin(TDR_SERIAL_BAUD);

    return true;
} // end of setup()

/*
 *  Function:       bool TDR::read()
 *  Description:    read the sensors
 */
bool TDR::read() {
    if( read_allTDR() == true ) {
        return true;
    } else {
        return false;
    }
} // end of read()

/*
 *  Function:       void TDR::print_data()
 *  Description:    print all data
 */
void TDR::print_data() {

    print_debug(data,                               // data where we store values
                time_data,                          // where we store time data
                TDR_num_of_variables,               // number of sensor we use
                counter_col,                        // counter for coloumn index
                counter_row,                        // counter for row index
                counter_col_overflow);              // flag for coloumn counter overflow

} // end of print_data()

/*
 *  Function:    TDR::exec_timer()
 *  Description: execute read if the timer is right
 */
bool TDR::exec_timer() {

    if(millis() - exec_timer_last >= exec_time) {
        
        if(read()) {
            print_data();
            #ifdef debug
                serial_debug.println("TDR (exec_timer) - executed read");
            #endif

            exec_timer_last = millis();

            return true;
        }
        return false;
        
    }

    return false;
} // end of exec_timer()

/*
 *  Function:       bool TDR::read_allTDR()
 *  Description:    read all TDR data
 */
bool TDR::read_allTDR() {

    // example:
    // 0+25.03+32.16+32.13+1600+1700\r\n
      
    String recv_string = "";                           // where to store string
    int TDR_counter = 0;                               // tdr counter for variables

    // sending ready to arduino
    delay(1000);

    // while we have some data 
    while(TDR_SERIAL.available() > 0) {

        int inChar = TDR_SERIAL.read();                 // read the serial

        if(inChar == '0' && TDR_counter == 0) {
            // the first 0
        } else {
            // detecting the + character
            if(inChar == '+') {

                // dependent on the  TDR_counter
                switch(TDR_counter) {
                    case 1:
                        current_vol_w_content = recv_string.toInt();
                        break;
                    case 2:
                        current_soil_temp = recv_string.toInt();
                        break;
                    case 3:
                        current_soil_perm = recv_string.toInt();
                        break;
                    case 4:
                        current_soil_elec = recv_string.toInt();
                        break;

                }

                TDR_counter++;                              // tdr var counter increment
                recv_string = "";                           // delete the recv_string because it has been processed above

            } else if(inChar == 114) {
                // \r - ignore
            } else if(inChar == 110) {
                
                // \n 

                current_other_data = recv_string.toInt();   // the last data
                
                break;

            } else {

                // if no symbol then just read the character as a digit
                if(isDigit(inChar)) {
                    recv_string += (char)inChar;
                }

            } // end of if
        }// end of if
    }// end of while

    return true;

} // end of read_allTDR()
