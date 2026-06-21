#ifndef __GPS_UART_H
#define __GPS_UART_H

#include "main.h"
#include <stdint.h>

#define GPS_UART_BUFFER_SIZE 128

void GPS_UART_Init(void);
void GPS_UART_IRQHandler(void);
uint8_t GPS_UART_GetSentence(char *sentence);

#endif // __GPS_UART_H
