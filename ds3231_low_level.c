/* DS3231 low level API - Reza Ebrahimi v1.0 */
#define F_CPU 16000000UL

#include "ds3231.h"
#include "i2c_lib_S25.h"
#include <util/delay.h>

/* function to transmit one byte of data to register_address on ds3231 (device_address: 0x68) */
void time_i2c_write_single(uint8_t device_address, uint8_t register_address, uint8_t *data_byte)
{
	/* Start write transaction */
	TWI_Address(device_address, TW_WRITE);
	/* Send register address */
	TWI_Transmit_Data(register_address);
	/* Send data byte */
	TWI_Transmit_Data(*data_byte);
	/* End transaction */
	TWI_Stop();
}

/* function to transmit an array of data to device_address, starting from start_register_address */
void time_i2c_write_multi(uint8_t device_address, uint8_t start_register_address, uint8_t *data_array, uint8_t data_length)
{
	// Start write transaction 
	TWI_Address(device_address, TW_WRITE);
	// Send start register address 
	TWI_Transmit_Data(start_register_address);
	// Send data bytes
	for (uint8_t i = 0; i < data_length; i++) {
		TWI_Transmit_Data(data_array[i]);
	}
	// End transaction 
	TWI_Stop();
}

/* function to read one byte of data from register_address on ds3231 */
void time_i2c_read_single(uint8_t device_address, uint8_t register_address, uint8_t *data_byte)
{
	// Set register pointer 
	TWI_Address(device_address, TW_WRITE);
	TWI_Transmit_Data(register_address);
	// Repeated start for read 
	TWI_Address(device_address, TW_READ);
	// Read data byte 
	*data_byte = TWI_Receive_Data();
	// End transaction
	TWI_Stop();
}

/* function to read an array of data from device_address */
void time_i2c_read_multi(uint8_t device_address, uint8_t start_register_address, uint8_t *data_array, uint8_t data_length)
{
	for (uint8_t i = 0; i < data_length; i++) {
		/* read one byte from register (start + i) */
		time_i2c_read_single(device_address,
		start_register_address + i,
		&data_array[i]);
	}
}

/* function to initialize I2C peripheral in 100kHz or 400kHz */
void ds3231_I2C_init()
{
	TWI_Host_Initialize();
}
