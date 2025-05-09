#ifndef I2C_H
#define I2C_H

#include <avr/io.h>  
#include <util//twi.h>

// Define the I2C pins
#define I2C_PORT     PORTA
#define I2C_SDA      PIN2_bm
#define I2C_SCL      PIN3_bm
#define TW_WRITE     0
#define TW_READ      1

void TWI_Stop();

void TWI_Host_Initialize();
    /*
        Pseudo Code
            Step 1: Set communication rate 
                -Given 
                    - SCLK = 80KHz 
                    - Rise Time = 10ns



            Step 2: Enable i2c



            Step 3: Set initial status to idle

            
    */


void TWI_Address(uint8_t Address, uint8_t mode);
   /*
        Psuedo Code
            Step 1: Shift Address left by 1


            Step 2: bitwise OR address with mode



            Step 3: Set the Address Register to updated address


            Step 4: Create a flag variable and set to correct interrupt flag based on mode
                - uint8_t flag;
                - check mode 0 = Read and 1 = Write
                - what interrupt flag would we use



            Step 5: Wait for the flag bit is set
            Use which works for you:
                - loop_until_bit_is_set(TWI0.MSTATUS, flag);
                - while(TWI0.MSTATUS & flag){}


            Step 6: Check if the client acknowledged (ACK) else keep infinite loop going trying to send address
                - If acknowledge you can call the STOP


            Step 7: Check for errors not there break out of infinite loop both have to be false
                - Arbitration Lost if false break infinite loop.
                - Bus Error if false break infinite loop


    */


int TWI_Transmit_Data(uint8_t data);
    /*
        Pseudo Code:
            Step 1: Start the data transfer by writing the data to register


            Step 2: Wait for the Write Interrupt Flag (WIF) to be set
            Use which works for you:
                - loop_until_bit_is_set(TWI0.MSTATUS, TWI_WIF_bp);
                - while(TWI0.MSTATUS & TWI_WIF_bp){}



            Step 3: Check for errors after the flag is set two errors to check include:
                - Arbitration Lost if true return -1.
                - Bus Error if true return -1
                - Otherwise, return 0 to indicate success.


    */



uint8_t TWI_Receive_Data();
    /*
        Pseudo Code:
            Step 1: Wait until the Read Interrupt Flag (RIF) is set 
            Use which works for you:
                - loop_until_bit_is_set(TWI0.MSTATUS, TWI_RIF_bp);
                - while(TWI0.MSTATUS & TWI_RIF_bp){}


            Step 2: Read data register into a variable to hold data
                - uint8_t data


            Step 3:  Respond with NACK 
                - TWI_ACKACT_bm


            Step 4: return data


    */

#endif 


