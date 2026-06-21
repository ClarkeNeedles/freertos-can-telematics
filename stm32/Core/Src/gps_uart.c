/*
  ***************************************************************************************************************
  ***************************************************************************************************************
  ***************************************************************************************************************

  File:		  gps_uart.c
  Author:     Clarke Needles
  Updated:    Nov 7, 2025

  ***************************************************************************************************************
*/

#include "gps_uart.h"
#include <string.h>

extern UART_HandleTypeDef huart2;
static uint8_t rxByte;
static char rxBuffer[GPS_UART_BUFFER_SIZE];
static uint16_t rxIndex = 0;
static char latestSentence[GPS_UART_BUFFER_SIZE];
static volatile uint8_t sentenceReady = 0;

/* To be used to initialize the GPS
 */
void GPS_UART_Init(void)
{
    rxIndex = 0;
    sentenceReady = 0;

    // Start receiving one byte at a time using interrupt
    HAL_UART_Receive_IT(&huart2, &rxByte, 1);
}

/* Called each time a new character is ready to be read from
 * the GPS
 */
void GPS_UART_IRQHandler(void)
{
	// Store byte in buffer
	if (rxIndex < GPS_UART_BUFFER_SIZE - 1)
	{
		rxBuffer[rxIndex++] = rxByte;
	}

	// Check for end of line
	if (rxByte == '\n')
	{
		// null terminate string
		rxBuffer[rxIndex] = '\0';
		strcpy(latestSentence, rxBuffer);
		// sentence is ready to be read
		sentenceReady = 1;
		// reset index for the next line
		rxIndex = 0;
	}

	// Continue receiving next byte
	HAL_UART_Receive_IT(&huart2, &rxByte, 1);
}

/* When a full sentence is ready, use this function
 * to get the entire sentence
 */
uint8_t GPS_UART_GetSentence(char *sentence)
{
    if (sentenceReady)
    {
        __disable_irq();
        strcpy(sentence, latestSentence);
        sentenceReady = 0;
        __enable_irq();

        return 1;
    }

    return 0;
}
