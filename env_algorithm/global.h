
#include <stdint.h>

#include <Arduino.h>

#define debug
#define serial_debug            Serial
#define serial_debug_speed      115200

void update_16bit(byte data[][8][8], 
                  byte time_data[8][8],
                  int var_id, 
                  int& col, 
                  int& row, 
                  bool& counter_col_overflow, 
                  int number_of_variables,
                  int value);

void print_debug(byte data[][8][8], 
                 byte time_data[8][8],
                 int number_of_variables,
                 int col,
                 int row,
                 bool counter_col_overflow);

float mapf(float x, float in_min, float in_max, float out_min, float out_max);


