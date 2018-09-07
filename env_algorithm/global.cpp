#include "global.h"

#include <Arduino.h>

/*
 *  Function: void update_16bit(...)
 *  Description: add `value` to your `data` array (converting to 16bit (2x8bit))
 *  Parameters:
 *      - uint8_t data[][8][8]          - the array where we store our data
 *      - int var_id                    - the variables ID (to find it in the array)
 *      - int* col                      - the coloumn variable (pointer because we modif it)
 *      - int* row                      - the row variable (pointer because we modif it)
 *      - bool* counter_col_overflow    - the counter coloumn overflown flag (set when we have filled all 64bits)
 *      - int value                     - the value we add to the data array
 */
void update_16bit(uint8_t data[][8][8], 
                  int time_data[8][8],
                  int var_id, 
                  int& col, 
                  int& row, 
                  bool& counter_col_overflow, 
                  int number_of_variables,
                  int value) {
    
    // writing the data
    data[var_id][col][row + (row + 0)]  = lowByte(value);
    data[var_id][col][row + (row + 1)]  = highByte(value);

    // get the seconds since 
    int now_time = int(millis() / 1000);

    // write the time data
    time_data[col][row + (row + 0)]     = lowByte(now_time);
    time_data[col][row + (row + 1)]     = highByte(now_time); 

    // if we are processing the last variable
    if(number_of_variables == var_id + 1) { 

        // increment the row
        row++;

        // if it is more than 3 (because we are fitting 4 data points 4x2 into 8bits)
        if(row > 3) {
            row = 0;                            // go back to null
            col++;                              // go to the next coloumn`
            
            // if we are overflowing
            if(col > 7) {
                col = 0;                        // go back 
                counter_col_overflow = 1;       // set the overflow flag
            } 
        }
    }
}

/*
 *  Function: void print_debug(...)
 *  Description: print the data array
 *  Parameters:  
 *      - uint8_t data[][8][8]          - the array we print from
 *      - int number_of_variables       - number of variables it has (first [] (we could add auto detection of this))
 *      - int col                       - the current coloumn counter
 *      - int row                       - the current row counter
 *      - bool counter_col_overflow     - if we have filled all 64bit
 */
void print_debug(uint8_t data[][8][8], 
                 int time_data[8][8],
                 int number_of_variables,
                 int col,
                 int row,
                 bool counter_col_overflow) {

    // go through all the variables
    for(int var=0; var < number_of_variables; var++) {
        
        // print the variable number 
        serial_debug.print("Variable num: ");serial_debug.println(var);

        // if coloumn counter overflown flag is set
        if(counter_col_overflow) {
            
            // col and row set at 8 so we send all 64bit
            col = 8;
            row = 8;
        } else {

            // increment col because of below for loop
            col++;
        }

        for(int c=0; c < col; c++) {
            
            serial_debug.print("Data[");serial_debug.print(c);serial_debug.print("]: ");
            for(int r=0; r < 8; r++) {
                serial_debug.print(data[var][c][r], HEX);
                serial_debug.print("(");serial_debug.print(time_data[c][r]);serial_debug.print("s) | ");
            }
            serial_debug.println();
        }

    }

    serial_debug.println();
}

/*
 *  Function:       float mapf(...)
 *  Description:    same as map but with floats
 */
float mapf(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}