/*
 * C_Soft_UART_Test1.c
 *
 * Created: 15.12.2022. 20:05:18
 * Author : Aleksandar Bogdanovic
 */ 


#ifndef F_CPU
#define F_CPU 16000000
#endif

#include <avr/io.h>
#include <util/delay.h>
#include "defines.h"

uint8_t received_data;
extern void startString();

int main(void)
{
	
    U_Init();
	
	startString();					// Salje Start...
	
    while (1) 
    {
		if (RxDone)
	
		{
			U_RxByte(&received_data);
			U_TxByte(received_data);
			
		}
    }
}
