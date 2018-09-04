# Explaining the code 
- all devices have ID, CANID and VAR_NUM
    - ID - the ID of the sensor (L0_ID 0)
    - CANID - the ID which is used to indentify the sensor through CAN (L0_CANID 0x101)
    - VAR_NUM - the number of variables we are using with the sensor (L0_VAR_NUM 2)

In the SENSOR DEFINES section we define all the necessary defines for the sensors like #define BQ34Z100_SERIESCELLS 3.

- const int number_of_sensor - this defines how many sensors are we reading. If it is set to 1 we will only look at the ID 0. If it is set to 4 then we will look at the sensors ID from 0 to 3.

- EXTRA DEFINES
    - #define debug - if it is defined we will get all debug messages
    - #define TIMER_SECOND - on every TIMER_SECOND second we will wake up
    
- PINS
    - these pins are used to set the CAN ID, which is not entirely implemented

- SERIAL
    - #define serial - here we define which serial we want to use (default: Serial)
    - #define SERIAL_SPEED - the baudrate of the serial

- CAN MODULE
    - all includes for the MCP and SPI libraries (which can be found in the Library folder)
    - #define CAN_PIN_INT - this is the pin for the interrupt of the MCP (default 4)
    - #define CAN_PIN_NSS - this is the chip-select pin of the MCP (default 9)
    - #define CAN_MASTER_ID - the CAN ID of the Raspberry Pi
    - #define CAN_ID - our base address (for example if it is 0x100 then all sensors are 0x101, 0x102, 0x103 ect.)
    - MCP_CAN CAN_BUS(CAN_PIN_NSS) - the CAN Bus object
    - bool send_via_int = false - this is the flag which allows the firmware to send when the interrupt is set, we use this to not have CAN Bus debouncing
    - bool exec_int_can = true - this flag allows the firmware to process the interrupt, this is set to false when we are processing our message in the loop()
    - bool not_my_id = false - this flag is for the filtering of the CAN Bus message ID
    - long unsigned int CAN_RXID - the received CAN Message ID

- Data
    - byte data[number_of_sensors][3][8][8] 
        - this is where we store all data
            - number_of_sensors - we init the array dependent on how many sensors we are using
            - 3 - the number of variables per device (we are using 3 here, because the max used is 3)(we could add auto detection of this number aswell)
            - [8][8] - the number of coloumns*rows (64bit)
            
    - bool init_worked[number_of_sensors];
        - this is where we store if our sensors have inited
    - int  data_index_coloumn[number_of_sensors] - the index counter of each sensor (COLOUMN)
    - int  data_index_row[number_of_sensors] - the index counter of each sensor (ROW)
    - bool data_coloumn_max[number_of_sensors] - this is set to true (for each sensor) if it has overflown the 64bit data
    - int shifted_first-second - when writing the data into the byte data array we need to shift the counter to fit larger numbers

- SENSORS
    - #include "TSL2561.h" - include the TSL2561 library
    - TSL2561 _TSL2561(TSL2561_ADDR_FLOAT) - initing the TSL2561 object 
    - #include "BME280.h" - include the BME280 library
    - BME280 _BME280(Wire,0x76) - init the object with Wire and address
    - #define _ANEMOMETER_PIN - the anemometer ADC pin (default A0)
    - #define _RAIN_PIN - the rain sensor digital pin (default 2)
    - volatile unsigned long rain_tipCount - the tip counter
    - volatile unsigned long rain_ContactTime - contact time for the deboucing
    - float rain_totalRainfall - total rain fall
    
- Other functions
    - mapf(float x, float in_min, float in_max, float out_min, float out_max) - same as map function but with floats
    - void print_data() - prints all data in the data array
    - bool init_sensor(int num) - executes the init sequency for the num sensor. This is where the init_worked[num] is set to true or false
    - void get_sensor_data(int num) - this is where we read the data from sensor num and put it into the data array
        - first collect the data
        - if debug is defined we print it out
        - we calculate the shifting if needed 
            -  shifted_first  = data_index_row[num];            
            - shifted_second = data_index_row[num] + 1; 
            -  [x1][y1][x2][y2][x3][y3][x4][y4]
            -  Example why we use this:
                - X: int(27.32) = 27
                - Y: int((27.32 - int(27.32)) * 100) = int(0.32 * 100) = int(32) = 32
        - write it into the data
            - data[num][num_counter][data_index_coloumn[num]][data_index_row[num] + shifted_first] = data
                - num - the sensor (by ID)
                - num_counter - the variable position
                - data_index_coloumn[num] - the number we are at the coloumn counter
                - data_index_row[num] - the number we are at the row counter
                - +shifted_first - the shifted value
        - data_index_row[num]++;
            - we increment the row number
        - out of reach
            - if(data_index_row[num] > 3) - we are out of rows (>3 because we have two data points -> 8/2)
            - data_index_row[num] = 0 - we go on the beginning of the row
            - data_index_coloumn[num]++ - go to the next coloumn
            - if(data_index_coloumn[num] > 7) - overflown the coloumn
            -  data_index_coloumn[num] = 0 - go back to the beginning of the coloumn
            -  data_coloumn_max[num] = 1 - set the overflown flag
            -  
        - WHEN SENDING BIG NUMBERS
            - this is where the shifting comes in handy the most, if you have 4 digits, you just shift 4 times
            
    - byte id_by_resistors()
        - this function reads the resisotr pins and defines the address
        - NOT TESTED!
    - void CAN_setup()
        - begins the CAN Bus by defined parameters
    
    - void sleep_devices(void)
        - set the CAN Bus and the STM32 to sleep
    
    - void setup()
        - serial.begin(SERIAL_SPEED) - begin the serial
        - CAN_setup() - start the CAN Bus 
        - for loop to init all sensors
    
    - void loop()
        - if we have the send_via_int flag set 
            -  exec_int_can = false - so we don't execute another cycle while we are processing (causes problems with timing)
            -  switch(CAN_RXID) - checks the received CAN Bus Message ID
                -  chosen_sensor - we choose the sensor
                -  get_sensor_data(x) - read the sensor data
                -  data_begin[1] - we put the number of variables here (this array is sent firt, so the master knows how many variables to expect)
            - if(data_coloumn_max[chosen_sensor] == true) - this is where we check if we have overflown the coloumn, if true, we know that we have to send 64bits independent on the row counter
                - data_begin[0]= 7 - expect all 8 coloumn
                - data_index_coloumn[chosen_sensor] = 7 - it will send all 8
            - else
                - data_begin[0] = data_index_coloumn[chosen_sensor] - send the expected value 
            - sndStat = CAN_BUS.sendMsgBuf(CAN_MASTER_ID, 0, 2, data_begin) - send the message
            -  for(int varz=0; varz<data_begin[1];varz++)
                - loops through all the variables (number of variables)
                - for(int i=0; i < data_index_coloumn[chosen_sensor]+1; i++)
                    - loops through all the coloumns
                    - if(i == data_index_coloumn[chosen_sensor] && data_coloumn_max[chosen_sensor] == 0)
                        - if it is not a full 8bit row we send
                    - else 
                        - we have all 8bits avaible, send that many
            - for(int i=0; i<number_of_sensors; i++)
                - loop through a number of sensor
                    - data_index_row[i]     = 0 - row to 0
                    - data_index_coloumn[i] = 0 - coloumn to 0
                    - data_coloumn_max[i]   = false - max flag set to false
            - memset(data, 0, sizeof(data)) - zero the whole data array
            - send_via_int = false - clear the interrupt flag
            - exec_int_can = true - allow interrupt on can
        - else - this is executed if we don't have anything on the CAN Bus
            - if(not_my_id == false && send_via_int == false)
                - not_my_id - must be false because we only execute when we read our ID
                - send_via_int must be also false because we don't execute two times (once on interrupt above, and then here again)
                - sleep_devices() - sleep the devices
                - loop through the sensor and get all data
    - void ISR_CAN() 
        - this executes on CAN Interrupt pin
        - STM32L0.wakeup() - we wakeup the processor
        - if(exec_int_can)
            - if we can execute via can
                - CAN_BUS.readMsgBuf(&CAN_RXID,NULL,NULL) - this is important because this is where we clear the interrupt flag
                - if((CAN_ID & CAN_RXID) == int(CAN_ID)) - if the CAN_RXID is in our ID range 
                    - send_via_int = true - we can send via interrupt (in loop())
                    - not_my_id = false - it is our id
            - else
                - this is not our ID
    - void ISR_RAIN() 
        - STM32L0.wakeup() - we wakeup the processor
        - if((millis() - rain_ContactTime) > 70)
            - debounce every 70mS
            - rain_tipCount++ - we increment the tip count
            - rain_ContactTime = millis() - for debounce
        - send_via_int = false;not_my_id = true;CAN_RXID = 0 - it this is not false we would read all sensors all again
        
            
