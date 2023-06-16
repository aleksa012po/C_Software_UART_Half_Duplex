/*
 * defines.h
 *
 * Created: 15.12.2022. 20:05:59
 *  Author: Aleksandar Bogdanovic
 */ 


#ifndef DEFINES_H_
#define DEFINES_H_

#include <avr/interrupt.h>
#include <avr/io.h>

#ifndef F_CPU
#define F_CPU 16000000
#endif

#define FALSE 0
#define TRUE !FALSE

/* Port and pin settings. */
#define U_DDR	DDRB				// Data direction register. Not available for high voltage ports.
#define U_PORT	PORTB				// Set port for communication.
#define U_PIN	PINB				// Set pin for communication.
#define U_RX	PB0					// Set Rx Pin
#define U_TX	PB1					// Set Tx Pin

/* UART settings */
#define BAUD 9600
#define BIT_TIME (uint16_t)(F_CPU / BAUD + 0.5)	//Clock for one bit (rounded)
#define RECEIVED_DATA_SIZE 8

extern volatile uint8_t	RxDone;

/* Global UART functions. */
void U_Init(void);									// Initialize the UART.
void U_RxByte(uint8_t *data);						// Receive byte data.
void U_TxByte(uint8_t data);						// Transmit byte data.
void U_RxBuffer(uint8_t *buffer, uint8_t length);	// Receive buffer.
void U_TxBuffer(uint8_t *buffer, uint8_t length);	// Transmit buffer.

#endif /* DEFINES_H_ */