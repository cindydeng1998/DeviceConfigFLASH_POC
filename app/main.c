/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include <stdio.h>

#include "tx_api.h"

#include "azure_iothub.h"
#include "networking.h"
#include "board_init.h"
#include "sntp_client.h"

#include "azure_config.h"

#define AZURE_THREAD_STACK_SIZE 4096
#define AZURE_THREAD_PRIORITY   4

#define FLASH_STORAGE 0x08080000
#define page_size 0x800

TX_THREAD azure_thread;
UCHAR azure_thread_stack[AZURE_THREAD_STACK_SIZE];

void azure_thread_entry(ULONG parameter);
void tx_application_define(void* first_unused_memory);

void save_to_flash(uint8_t *data)
{
	volatile uint64_t data_to_FLASH[(strlen((char*)data) / 8) + (int)((strlen((char*)data) % 8) != 0)];
	memset((uint8_t*)data_to_FLASH, 0, strlen((char*)data_to_FLASH));
	strcpy((char*)data_to_FLASH, (char*)data);

	volatile uint32_t data_length = (strlen((char*)data_to_FLASH) / 8) + (int)((strlen((char*)data_to_FLASH) % 8) != 0);
	volatile uint16_t pages = (strlen((char*)data) / page_size) + (int)((strlen((char*)data) % page_size) != 0);
	
	printf("data_len: %lu\n", data_length);
	
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
			if(status ==  HAL_OK)
			{
				write_cnt += 8;
				index++;
			}
		}
	}
	
	HAL_FLASH_OB_Lock();
	HAL_FLASH_Lock();		
		
}

void azure_thread_entry(ULONG parameter)
{
	char testData[50];
	
	memset(testData, 0, sizeof(testData));
	strcpy(testData, "Hello World");
	
	printf("%s", testData);
	
	save_to_flash((uint8_t*)testData);	
		
    UINT status;

    printf("\r\nStarting Azure thread\r\n\r\n");

    // Initialize the network
    if (stm32_network_init(WIFI_SSID, WIFI_PASSWORD, WIFI_MODE) != NX_SUCCESS)
    {
        printf("Failed to initialize the network\r\n");
        return;
    }

    // Start the SNTP client
    status = sntp_start();
    if (status != NX_SUCCESS)
    {
        printf("Failed to start the SNTP client (0x%02x)\r\n", status);
        return;
    }

    // Wait for an SNTP sync
    status = sntp_sync_wait();
    if (status != NX_SUCCESS)
    {
        printf("Failed to start sync SNTP time (0x%02x)\r\n", status);
        return;
    }

    // Enter the Azure MQTT loop
    if(!azure_iothub_run(IOT_HUB_HOSTNAME, IOT_DEVICE_ID, IOT_PRIMARY_KEY))
    {
        printf("Failed to start Azure IotHub\r\n");
        return;
    }
}

void tx_application_define(void* first_unused_memory)
{
    // Override the SYSTICK interval with the current clock
    SysTick->LOAD  = SystemCoreClock / TX_TIMER_TICKS_PER_SECOND - 1;

    // Create Azure thread
    UINT status = tx_thread_create(
        &azure_thread, "Azure Thread",
        azure_thread_entry, 0,
        azure_thread_stack, AZURE_THREAD_STACK_SIZE,
        AZURE_THREAD_PRIORITY, AZURE_THREAD_PRIORITY,
        TX_NO_TIME_SLICE, TX_AUTO_START);

    if (status != TX_SUCCESS)
    {
        printf("Azure IoT thread creation failed\r\n");
    }
}



/*
void read_flash(uint8_t* data)
{
	volatile uint32_t read_data;
	volatile uint32_t read_cnt = 0;
	
	do
	{
		read_data = *(uint32_t*)(FLASH_STORAGE + read_cnt);
		if (read_data != 0xFFFFFFFF)
		{
			data[read_cnt] = (uint8_t)read_data;
			data[read_cnt + 1] = (uint8_t)(read_data >> 8);
			data[read_cnt + 2] = (uint8_t)(read_data >> 16);
			data[read_cnt + 3] = (uint8_t)(read_data >> 24);
			read_cnt += 4;
		}
	} while (read_data != 0xFFFFFFFF);
	
	
}
*/


int main(void)
{
    // Initialize the board
    board_init();

    // Enter the ThreadX kernel
    tx_kernel_enter();
    return 0;
}
