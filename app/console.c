/* Copyright (c) Microsoft Corporation.
   Licensed under the MIT License. */

#include "stm32l4xx_hal.h"

#include "board_init.h"

/* Define buffer size. Buffer size must be power of 2.  */
#define CONSOLE_TX_BUFFER_SIZE  128
#define CONSOLE_RX_BUFFER_SIZE  128

static uint8_t console_rx_buffer[CONSOLE_RX_BUFFER_SIZE];
static uint32_t console_rx_buffer_cindex;
static volatile uint32_t console_rx_buffer_pindex;
uint32_t console_rx_buffer_overflow;


int __io_putchar(int ch);
int __io_getchar(void);
int _read(int file, char *ptr, int len);
int _write(int file, char *ptr, int len);

int __io_putchar(int ch)
{
	HAL_UART_Transmit(&UartHandle, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
	return ch;
}

int __io_getchar(void)
{
	uint8_t ch;
	
	HAL_UART_Receive(&UartHandle, &ch, 1, 1); // works but times out after a while
	
//	/* Echo character back to console */
//	HAL_UART_Transmit(&UartHandle, &ch, 1, HAL_MAX_DELAY);
		
	/* And cope with Windows */
	if (ch == '\r') {
		uint8_t ret = '\n';
		HAL_UART_Transmit(&UartHandle, &ret, 1, HAL_MAX_DELAY);
	}

	return ch;
}

int _read(int file, char *ptr, int len)
{
	int DataIdx;

	for (DataIdx = 0; DataIdx < len; DataIdx++)
	{
		*ptr++ = __io_getchar();
	}

	return len;
}

int _write(int file, char *ptr, int len)
{
	int DataIdx;

	for (DataIdx = 0; DataIdx < len; DataIdx++)
	{
		__io_putchar(*ptr++);
	}
	return len;
}

//void USART1_IRQHandler(void)
//{
//	/* Check RXNE flag.  */
//	if (__HAL_UART_GET_FLAG(&UartHandle, UART_FLAG_RXNE))
//	{
//        
//		/* Check if buffer is available.  */
//		if (((console_rx_buffer_pindex + 1) & (CONSOLE_RX_BUFFER_SIZE - 1)) != console_rx_buffer_cindex)
//		{
//			/* Read data into buffer.  */
//			uint8_t ch;
//			ch = (uint8_t)UartHandle.Instance->RDR;
//			console_rx_buffer[console_rx_buffer_pindex] = ch;
//			console_rx_buffer_pindex = (console_rx_buffer_pindex + 1) & (CONSOLE_RX_BUFFER_SIZE - 1);
//		}
//		else
//		{
//            
//			/* Discard data due to buffer overflow.  */
//			UartHandle.Instance->RDR;
//			console_rx_buffer_overflow++;
//		}
//	}
//    
//	/* Check ORE flag. In case a overflow condition is hit just after status is read.  */
//	if (__HAL_UART_GET_FLAG(&UartHandle, UART_FLAG_ORE))
//	{
//        
//		/* Discard data due to buffer overflow.  */
//		UartHandle.Instance->RDR;
//		console_rx_buffer_overflow++;
//	}
//}