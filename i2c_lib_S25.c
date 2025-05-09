#include "i2c_lib_S25.h"
#include <avr/sfr_defs.h>

void TWI_Stop()
{
	TWI0.MCTRLB |= TWI_MCMD_STOP_gc;
}


void TWI_Host_Initialize()
{
	// Step 1: Set communication rate
	// SCLK = 80 KHz, Rise Time = 10ns
	// TWI0.MBAUD = 95;
	
	// SCLK = 100 KHz, Rise Time = 10ns
	TWI0.MBAUD = 70;
	
	// Step 2: Enable i2c
	TWI0.MCTRLA |= TWI_ENABLE_bm;
	
	// Step 3: Set initial status to idle
	TWI0.MSTATUS |= TWI_BUSSTATE_IDLE_gc;
		
}

void TWI_Address(uint8_t Address, uint8_t mode)
{
	while (1) {
		// Step 1: Shift Address left by 1
		uint8_t addressWithMode = Address << 1;
	
		// Step bitwise OR address with mode
		addressWithMode |= mode;
	
		// Step 3: Set the Address Register to updated address
		TWI0.MADDR = addressWithMode;
	
		// Step 4: Create a flag variable and set to correct interrupt flag based on mode
		uint8_t flag;
		if (mode == TW_WRITE) {
			flag = TWI_WIF_bp;
		}
		else {
			flag = TWI_RIF_bp;
		}
	            
		// Step 5: Wait for the flag bit is set
		loop_until_bit_is_set(TWI0.MSTATUS, flag);
					
		// Step 6:  Check if the client acknowledged (ACK) else keep infinite loop going trying to send address
		// - If acknowledge you can call the STOP
		if (TWI0.MSTATUS & TWI_RXACK_bm) {
			TWI_Stop();
		}
	
		// Step 7: Check for errors not there break out of infinite loop both have to be false
		// - Arbitration Lost if false break infinite loop.
		// - Bus Error if false break infinite loop
		if (!(TWI0.MSTATUS & TWI_ARBLOST_bm) && !(TWI0.MSTATUS & TWI_BUSERR_bm)) {
			break;
		}
	}
	
	

}

int TWI_Transmit_Data(uint8_t data) 
{
	// Step 1: Write data to the register
	TWI0.MDATA = data;
	
	// Step 2: Wait for the Write Interrupt Flag to be set
	loop_until_bit_is_set(TWI0.MSTATUS, TWI_WIF_bp);
	
	// Step 3: Check for errors after the flag is set two errors to check include:
	// - Arbitration Lost return -1
	// - Bus Error return -1
	// - Otherwise, return 0 for success
	if (TWI0.MSTATUS & TWI_ARBLOST_bm || TWI0.MSTATUS & TWI_BUSERR_bm) {
		return -1;
	}
	
	return 0;
}

uint8_t TWI_Receive_Data()
{
	// Step 1: Wait until the Read Interrupt Flag (RIF) is set
	loop_until_bit_is_set(TWI0.MSTATUS, TWI_RIF_bp);
	
	// Step 2: Read data register into a variable to hold data
	uint8_t data = TWI0.MDATA;
	
	// Step 3: Respond with NACK
	TWI0.MCTRLB |= TWI_ACKACT_bm;
	
	// Step 4: return data
	return data;
}
