/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "device_config.h"

#define FLASH_STORAGE 0x08080000
#define page_size 0x800

void save_to_flash_ST(uint8_t *data);
void read_flash_ST(uint8_t* data);
HAL_StatusTypeDef erase_flash_ST();

void save_to_flash_ST(uint8_t *data)
{
	volatile uint64_t data_to_FLASH[(strlen((char*)data) / 8) + (int)((strlen((char*)data) % 8) != 0)];
	memset((uint8_t*)data_to_FLASH, 0, strlen((char*)data_to_FLASH));
	strcpy((char*)data_to_FLASH, (char*)data);

	volatile uint32_t data_length = (strlen((char*)data_to_FLASH) / 8) + (int)((strlen((char*)data_to_FLASH) % 8) != 0);
	volatile uint16_t pages = (strlen((char*)data) / page_size) + (int)((strlen((char*)data) % page_size) != 0);
		
	// unlock flash
	HAL_FLASH_Unlock();
	HAL_FLASH_OB_Unlock();
	
	FLASH_EraseInitTypeDef EraseInitStruct;
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.Banks = FLASH_BANK_2;
	EraseInitStruct.Page = 0; // bank 2 page 0 is 0x80800000
	EraseInitStruct.NbPages = pages;
	
	
	volatile uint32_t write_cnt = 0;
	volatile uint32_t index = 0;
	volatile HAL_StatusTypeDef status;
	uint32_t PageError;
	
	status = HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
	
	while (index < data_length)
	{
		if (status == HAL_OK)
		{
			status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, FLASH_STORAGE + write_cnt, data_to_FLASH[index]); // FAST
			HAL_FLASH_GetError();
			if (status ==  HAL_OK)
			{
				write_cnt += 8;
				index++;
			}
		}
	}
	
	// Lock flash
	HAL_FLASH_OB_Lock();
	HAL_FLASH_Lock();		
		
}

int8_t save_to_flash(char *hostname, char *device_id, char* primary_key)
{
	int8_t ret_val = 1;
	
	const char *format = "hostname=%s device_id=%s primary_key=%s";
	
	char writeData[MAX_READ_BUFF] = { 0 };
	
	// Create credential string using format string
	if(sprintf(writeData, format, hostname, device_id, primary_key) < 0)
	{
		// Error 
		ret_val = 0;
		return ret_val;
	}
	
	// Call device specific implementation of FLASH storage
	save_to_flash_ST((uint8_t *)(writeData));
	
	return ret_val;

}

void read_flash(char* hostname, char* device_id, char* primary_key)
{
	char readData[MAX_READ_BUFF] = { 0 };
	char _hostname[MAX_HOSTNAME_LEN] = { 0 }; 
	char _device_id[MAX_DEVICEID_LEN] = { 0 };
	char _primary_key[MAX_KEY_LEN] = { 0 };

	const char *format = "hostname=%s device_id=%s primary_key=%s"; 
	
	// Call MCU specific flash erasing function
	read_flash_ST((uint8_t*)(readData));

	// Parse credentials from string
	sscanf(readData, format, _hostname, _device_id, _primary_key);
	
	// Store content in buffers
	strcpy(hostname, _hostname);
	strcpy(device_id, _device_id);
	strcpy(primary_key, _primary_key);
}


void read_flash_ST(uint8_t* data)
{
	volatile uint32_t read_data;
	volatile uint32_t read_cnt = 0;
	
	do
	{
		read_data = *(uint32_t*)(FLASH_STORAGE + read_cnt);
		if (read_data != 0xFFFFFFFF)
		{
			data[read_cnt] = (uint8_t)read_data;
			data[read_cnt + 1] = (uint8_t)(read_data >> 0x8);
			data[read_cnt + 2] = (uint8_t)(read_data >> 0x10);
			data[read_cnt + 3] = (uint8_t)(read_data >> 0x18);
			read_cnt += 4;
		}
		
	} while (read_data != 0xFFFFFFFF); // end of flash content
}


void verify_mem_status(void)
{
	// how to verify the memory is valid
	
}

bool has_credentials(void)
{
	bool ret_val = true;
	volatile uint32_t read_data;
	
	read_data = *(uint32_t*)(FLASH_STORAGE);
	
	if (read_data == 0xFFFFFFFF)
	{
		ret_val = false;
	}
	
	return ret_val;
}

void erase_flash()
{
	HAL_StatusTypeDef status;
	status = erase_flash_ST(); // would need return error type
}

HAL_StatusTypeDef erase_flash_ST()
{
	// unlock flash
	HAL_FLASH_Unlock();
	HAL_FLASH_OB_Unlock();
	
	FLASH_EraseInitTypeDef EraseInitStruct;
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.Banks = FLASH_BANK_2;
	EraseInitStruct.Page = 0; // bank 2 page 0 is 0x80800000
	EraseInitStruct.NbPages = 0x1;
	
	volatile HAL_StatusTypeDef status;
	uint32_t PageError;
	
	status = HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
	HAL_FLASH_GetError();
	
	// Lock flash
	HAL_FLASH_OB_Lock();
	HAL_FLASH_Lock();		
	
	return status;
}