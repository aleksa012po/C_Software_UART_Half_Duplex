/*
 * uart_setup.c
 *
 * Created: 15.12.2022. 20:13:21
 *  Author: Aleksandar Bogdanovic
 */ 

#include "defines.h"
#include <util/delay.h>
#include <string.h>
#include <stdbool.h>

volatile uint8_t RxDone;						// Ready to read Rx byte

static volatile uint8_t TxCount;				// 
static volatile uint8_t TxData;					// 
static volatile uint8_t RxData;					// 
static volatile uint8_t RxMask;					// 
static volatile uint8_t RxTemp;					//

volatile bool fallingEdge;
volatile unsigned long pulseCount;
volatile unsigned long overflowCount;
volatile unsigned long startTime;
volatile unsigned long finishTime;
volatile unsigned long minimumTime; 

/* TIMER1 Input Capture Interrupt Handler */

/* When Rx Pin detects start bit, interrupt handler is called, when than happens, 
   value of TCNT1 is written in ICR1 register 
Introduces an interrupt handler function (ISR) that runs with global 
interrupts initially disabled by default with no attributes specified. */

ISR(TIMER1_CAPT_vect) 
{
	/* We setup OCR1B to read Rx */
	OCR1B = ICR1 + (uint16_t)(BIT_TIME * 1.5);		// Output Compare B interrupt ce se dogoditi 1.5 BIT_TIME kasnije
	RxTemp = 0;										// Cisti privremenu promenljivu
	RxMask = 1;										// Postavlja bit masku da cita prvi bit
	
	TIFR1 = (1 << OCF1B);							// Cisti Output Compare B interrupt flag
	
	if (!(U_PIN & (1 << U_RX)))						// Ako je RX pin low, to znaci start bit
	{
		TIMSK1 = (1 << OCIE1A) | (1 << OCIE1B);		// Disable input capture (detektovanje start bit-a), Enable Output Compare A(za Tx) / B(za Rx) interrupt
	}
}

/* TIMER1 Output Compare B Interrupt Handler */

/* This handler is activated for reading data when we detect start bit.
   RxMask is TRUE while we read 8 bits (1 byte) from Rx Pin */

ISR(TIMER1_COMPB_vect)
 {
	if(RxMask) 
	{
		if (U_PIN & (1 << U_RX)) RxTemp |= RxMask;	// Ako je Rx input high, upisuje 1 u bit
		
		RxMask <<= 1;								// Shift 1 bit <<
		
		OCR1B += BIT_TIME;							// Postavlja OCR1B da cita sledeci bit	
	}
	else
	{
		RxDone = 1;									// Citanje 8 bitova je zavrseno
		RxData = RxTemp;							// Cuva Rx data
		TIFR1 = (1 << ICF1);						// Cisti input capture interrupt flag da cita novi bajt
		
		TIMSK1 = (1 << ICIE1) | (1 << OCIE1A);		// Ukljucuje input capture + COMPA interrupt
	}	
}

/* TIMER1 Output Compare A Interrupt Handler */

/* This handler is activated for sending data. CAUTION - in TxData memory, data is inverted. */

ISR(TIMER1_COMPA_vect)
{
	uint8_t Output;
	
	OCR1A += BIT_TIME;								// Same as OCR1A = OCR1A + BIT_TIME, Postavlja OCR1A da salje sledeci bit
	
	if (TxCount)									// Ako postoje bitovi za slanje
	{
		TxCount--;									// Umanji brojac
		
		Output = (1 << COM1A1);						// Put output as LOW
		
		if (TxCount != 9)							// Ako je bit podataka
		{
			if(!(TxData & 0x01)) Output = (1 << COM1A1) | (1 << COM1A0);							  // same as: if(TxData & 0x01 == 0), ako je bit LOW, postavi output HIGH (sTxData je invertovan)
			TxData >>= 1;							// Shift 1 bit >>
		}
		TCCR1A = Output;
	}
	
	
}

/* Software UART Inicijalizacija */
/* Ukljucuje Output Compare A interrupt da salje podatke					 
   Ukljucuje Input Capture interrupt da prima podatke  */

void U_Init()
{
	OCR1A = TCNT1 + 1;								//Output Compare A interrupt je pozvan odmah nakon inicijalizacija
	
	TCCR1A = (1 << COM1A1) | (1 << COM1A0);			// Postavlja output mode, postavlja OC1A high(Idle), Timer1 mode 0
	
	TCCR1B = (1 << ICNC1) | (1 << CS10);			// Input capture noise canceler, falling edge detection, no prescaling
	
	TIFR1 = (1 << ICF1);							// Cisti capture flag
	
	TIMSK1 = (1 << ICIE1) | (1 << OCIE1A);			// Ukljucuje input capture interrupt(Detektuje start bit) + Output compare A match interrupt(Tx)
	
	
	TxCount = 0;									// Nema Tx podataka
	RxDone = 0;										// Nema Rx podataka
	
	U_DDR |= (1 << U_TX);							// Postavlja TX pin kao output
	
	sei();											// Ukljucuje global interrupt
}

/* Cita bajt - 8 bits */
void U_RxByte(uint8_t *data)
{
	while( !RxDone );
	
	RxDone = 0;
	*data = RxData;
}

/* Upisuje bajt - 8 bita */
void U_TxByte(uint8_t data)
{
	while(TxCount);
	
	TxData = ~data;									// Upisuje invertovane podatke za generisanje stop bita
	TxCount = 10;									// Start(1) + data(8) + stop(1)
}

/* Cita bajtove */
void U_RxBuffer(uint8_t *buffer, uint8_t length)
{
	for (uint8_t i = 0; i < length; i++)
	{
		U_RxByte(buffer);
		buffer++;
	}
}

/* Upisuje bajtove */
void U_TxBuffer(uint8_t *buffer, uint8_t length)
{
	for (uint8_t i = 0; i < length; i++)
	{
		U_TxByte(*buffer);
		buffer++;
	}
}

/* Start string */
void startString(char c[], uint8_t mem)
{
	mem = 9;
	for (uint8_t a = 0; a < mem; a++)
	{
		char c[] = " Test... ";
		U_TxByte(c[a]);
	}
}
