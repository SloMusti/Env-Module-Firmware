
#include <stdint.h>

#define debug
#define serial_debug            Serial
#define serial_debug_speed      115200

void update_16bit(uint8_t data[][8][8], 
                  int time_data[8][8],
                  int var_id, 
                  int& col, 
                  int& row, 
                  bool& counter_col_overflow, 
                  int number_of_variables,
                  int value);

void print_debug(uint8_t data[][8][8], 
                 int time_data[8][8],
                 int number_of_variables,
                 int col,
                 int row,
                 bool counter_col_overflow);

