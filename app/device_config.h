/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

// ============================= Microchip specific =============================

//#include "atmel_start.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "stm32l4xx_hal.h"


//
///* A specific byte pattern stored at the begining of SmartEEPROM data area.
// * When the application comes from a reset, if it finds this signature,
// * the assumption is that the SmartEEPROM has some valid data.
// */
//#define SMEE_CUSTOM_SIG 0x5a5a5a5a
//
///* This address must be within the total number of EEPROM data bytes 
// * ( Defined by SBLK and PSZ fuse values).
// */
//#define SEEP_TEST_ADDR 32
//
///* This example assumes SBLK = 1 and PSZ = 3, thus total size is 4096 bytes */
//#define SEEP_FINAL_BYTE_INDEX 4095
//
//// Address of FLASH, spaced away from the verify signature
//#define FLASH_ADDRESS		(SEEPROM_ADDR + 0x8)
//
//#define FLASH_STORAGE 0x08080000
//
//#define page_size 0x800

// ============================= Microchip specific =============================


// Max length of the data buffer read from flash
#define MAX_READ_BUFF 380

// Max length of the Azure IoTHub hostname
#define MAX_HOSTNAME_LEN 100

// Max length of the device ID
#define MAX_DEVICEID_LEN 130

// Max length of the device primary key
#define MAX_KEY_LEN 150

// Value of empty EEPROM byte
#define EMPTY_EEPROM_VAL 0xFF


/* Handler for a HardFault */
void HardFault_Handler(void);

/* Checks if memory contains Azure IoT credentials*/
int8_t has_credentials(void);

/* Checks if memory is ready to use */
void verify_mem_status(void);

/* Save Azure IoT credentials to flash */
int8_t save_to_flash(char *hostname, char *device_id, char* primary_key);

/* Erase Azure IoT credentials from flash */ 
void erase_flash(void);

/* Read Azure IoT credentials from flash */
void read_flash(char* hostname, char* device_id, char* primary_key);


