#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

/* _____PROJECT INCLUDES_____________________________________________________ */
#include "uart0.h"

/**
 *  @name Size of transmit and receive buffer
 *
 *  @note Code will be optimized if the size is a power of two
 *        (2,4,8,16,32,64, 128 or 256).
 */
//@{
#ifndef UART0_TX_BUFFER_SIZE
#define UART0_TX_BUFFER_SIZE 32
#endif

#ifndef UART0_RX_BUFFER_SIZE
#define UART0_RX_BUFFER_SIZE 32
#endif
//@}

/* _____MACROS_______________________________________________________________ */
//! Macro to convert the specified BAUD rate to a 16-bit UBBR register value
#define UART0_UBBR_VALUE(Baud) (DIV(F_CPU, (16ul*Baud))-1)

/* _____LOCAL VARIABLES______________________________________________________ */
/// Receive ring (circular) buffer
static uint8_t UART0_u8RxBuffer[UART0_RX_BUFFER_SIZE];
static uint8_t UART0_u8RxOut;
static volatile uint8_t UART0_u8RxIn;

/// Transmit ring (circular) buffer
static uint8_t UART0_u8TxBuffer[UART0_TX_BUFFER_SIZE];
static volatile uint8_t UART0_u8TxOut;
static uint8_t UART0_u8TxIn;

/// Flag that is used by interrupt handler to indicate that transmission is finished (transmit buffer empty)
static bool  UART0_bTxFinishedFlag;

/// UART0 Rx Buffer Overflow
static void (* UART0_RxBuffOvf)(void);

/// UART0 Tx Buffer Overflow
static void (* UART0_TxBuffOvf)(void);

//! UART0 Rx Data Error
static void (* UART0_RxDataErr)(UART0_Error_t eError);

/* _____PRIVATE FUNCTIONS____________________________________________________ */
//! Received data interrupt handler
ISR(USART_RX_vect) {
	uint8_t ucsra = UCSR0A;
	uint8_t data  = UDR0;
	uint8_t index = UART0_u8RxIn;

	// Calculate next pointer position
	GET_NEXT_BUFF_IDX(index, UART0_RX_BUFFER_SIZE);

	// Make sure there is space available in buffer.
	if(index == UART0_u8RxOut) {
		if (UART0_RxBuffOvf != NULL)
			UART0_RxBuffOvf();

		// Receive buffer is full, discard received data
		return;
	}

	// Accept data only if there were no Framing, Data Overrun or Parity Error(s)
	if (ucsra & ((1 << FE0) | (1 << DOR0) | (1 << UPE0))) {
		if (UART0_RxDataErr != NULL) {
			if (ucsra & (1 << FE0))
				UART0_RxDataErr(UART0_FRAMING_ERROR);

			if (ucsra & (1 << DOR0))
				UART0_RxDataErr(UART0_DATA_OVERRUN_ERROR);

			if (ucsra & (1 << UPE0))
				UART0_RxDataErr(UART0_PARITY_ERROR);
		}

		// Received data had an error, discard received data
		return;
	}

	// Add data to ring buffer
	UART0_u8RxBuffer[UART0_u8RxIn] = data;

	// Advance pointer
	UART0_u8RxIn = index;
}

//! Transmit data register empty interrupt handler
ISR(USART_UDRE_vect)
{
	// See if there is more data to be sent
	if(UART0_u8TxOut == UART0_u8TxIn)
	{
		// Disable transmit data register empty interrupt
		BIT_SET_LO(UCSR0B, UDRIE0);

		// Enable transmit complete interrupt
		BIT_SET_HI(UCSR0B, TXCIE0);

		return;
	}

	// Clear flag to indicate that transmission is busy
	UART0_bTxFinishedFlag = FALSE;

	// Send data
	UDR0 = UART0_u8TxBuffer[UART0_u8TxOut];

	// Advance pointer
	GET_NEXT_BUFF_IDX(UART0_u8TxOut, UART0_TX_BUFFER_SIZE);
}

//! Transmit complete interrupt handler
ISR(USART_TX_vect) {
	// Set flag to indicate that transmission has finished
	UART0_bTxFinishedFlag = TRUE;

	// Disable interrupt
	BIT_SET_LO(UCSR0B, TXCIE0);
}

/* _____PUBLIC FUNCTIONS_____________________________________________________ */

void UART0_vInit(uint32_t baud, uint8_t data_bits, UART0_Parity_t parity, uint8_t stop_bits) {
	uint8_t ucsrc = 0x00;

	// Initialise variables
	UART0_u8RxIn    = 0;
	UART0_u8RxOut   = 0;
	UART0_u8TxIn    = 0;
	UART0_u8TxOut   = 0;
	UART0_bTxFinishedFlag = TRUE;

	UART0_RxBuffOvf = NULL;
	UART0_TxBuffOvf = NULL;
	UART0_RxDataErr = NULL;

	switch(parity) {
	case UART0_ODD_PARITY :
		// Odd parity
		ucsrc |= (1<<UPM01) | (1<<UPM00);
		break;
	case UART0_EVEN_PARITY :
		// Even parity
		ucsrc |= (1<<UPM01) | (0<<UPM00);
		break;
	case UART0_NO_PARITY :
		// Fall through...
	default:
		// No parity
		ucsrc |= (0<<UPM01) | (0<<UPM00);
		break;
	}

	switch(data_bits) {
	case 5:
		// 5 data bits
		ucsrc |= (0<<UCSZ02) | (0<<UCSZ01) | (0<<UCSZ00);
		break;
	case 6:
		// 6 data bits
		ucsrc |= (0<<UCSZ02) | (0<<UCSZ01) | (1<<UCSZ00);
		break;
	case 7:
		// 7 data bits
		ucsrc |= (0<<UCSZ02) | (1<<UCSZ01) | (0<<UCSZ00);
		break;
	case 8:
		// Fall through...
	default:
		// 8 data bits
		ucsrc |= (0<<UCSZ02) | (1<<UCSZ01) | (1<<UCSZ00);
		break;
	}

	UCSR0C = ucsrc;

	UART0_vChangeBaud(baud);
}


void UART0_vSetRxBuffOvfHandler(void (*UART0_RxBuffOvf_func)(void)) {
	UART0_RxBuffOvf = UART0_RxBuffOvf_func;
}

void UART0_vSetTxBuffOvfHandler(void (*UART0_TxBuffOvf_func)(void)) {
	UART0_TxBuffOvf = UART0_TxBuffOvf_func;
}

void UART0_vSetRxDataErrHandler(void (*UART0_RxDataErr_func)(UART0_Error_t eError)) {
	UART0_RxDataErr = UART0_RxDataErr_func;
}

void UART0_vChangeBaud(uint32_t baud) {
	uint16_t  ubrr;
	ldiv_t div;

	// Disable UART
	UCSR0B = 0;

	// Calculate new 16-bit UBBR register value
	baud <<= 4;
	div    = ldiv(F_CPU, baud);
	ubrr   = (uint16_t)div.quot;
	baud >>= 1;
	if((uint32_t)(div.rem) < baud) {
		ubrr--;
	}
	// Set BAUD rate by initalising 16-bit UBBR register
	UBRR0H = U16_HI(ubrr);
	UBRR0L = U16_LO(ubrr);

	// Enable RxD/TxD and receive interupt
	UCSR0B = (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0);
}

/**
 *  See if there is received data in the receive buffer.
 *
 *  @retval TRUE    There is received data in the receive buffer
 *  @retval FALSE   The receive buffer is empty
 */
bool UART0_bRxBufferEmpty(void) {
	// See if there is data in the buffer
	if(UART0_u8RxOut == UART0_u8RxIn) {
		return FALSE;
	}
	else {
		return TRUE;
	}
}


/**
 *  See if received byte is available and store it in specified location.
 *
 *  @param[out] pu8Data Pointer to location where data byte must be stored
 *
 *  @retval TRUE  Received byte is stored in specified location
 *  @retval FALSE No received data available (receive buffer empty)
 */
bool UART0_bGetRxByte(uint8_t* pu8Data) {
	// See if there is data in the buffer
	if(UART0_u8RxOut == UART0_u8RxIn) {
		return FALSE;
	}
	// Fetch data
	*pu8Data = UART0_u8RxBuffer[UART0_u8RxOut];

	// Advance pointer
	GET_NEXT_BUFF_IDX(UART0_u8RxOut,UART0_RX_BUFFER_SIZE);

	return TRUE;
}

/**
 *  Copy received data from ring buffer into specified buffer.
 *
 *  @param[out] pu8Buffer          Buffer to copy received data into
 *  @param[in] u8MaximumBufferSize Maximum number of received bytes to copy into buffer
 *
 *  @return uint8 Number of received bytes copied into buffer
 */
uint8_t UART0_u8GetRxData(uint8_t* pu8Buffer, uint8_t u8MaximumBufferSize) {
	uint8_t u8DataReceived = 0;

	while(u8MaximumBufferSize) {
		// See if there is data in the buffer
		if(UART0_u8RxOut == UART0_u8RxIn) {
			break;
		}

		// Fetch data
		*pu8Buffer++ = UART0_u8RxBuffer[UART0_u8RxOut];
		// Advance pointer
		GET_NEXT_BUFF_IDX(UART0_u8RxOut,UART0_RX_BUFFER_SIZE);
		// Next byte
		u8DataReceived++;
		u8MaximumBufferSize--;
	}
	return u8DataReceived;
}

uint8_t UART0_u8NoOfRxBytes(void) {
	if (UART0_u8RxIn >= UART0_u8RxOut) {
		return (UART0_u8RxIn - UART0_u8RxOut);
	} else {
		return ((UART0_RX_BUFFER_SIZE - UART0_u8RxOut) + UART0_u8RxIn);
	}
}

void UART0_vFlushRxData(void) {
	UART0_u8RxOut = UART0_u8RxIn = 0;
}

uint8_t UART0_u8NoOfTxBytes(void) {
	if (UART0_u8TxIn >= UART0_u8TxOut) {
		return (UART0_u8TxIn - UART0_u8TxOut);
	} else {
		return ((UART0_TX_BUFFER_SIZE - UART0_u8TxOut) + UART0_u8TxIn);
	}
}

uint8_t UART0_u8NoOfFreeTxBufferBytes(void) {
	return UART0_TX_BUFFER_SIZE - UART0_u8NoOfTxBytes() - 1;
}

bool UART0_bTxBufferFull(void) {
	uint8_t index = UART0_u8TxIn;

	// Calculate next pointer position
	GET_NEXT_BUFF_IDX(index, UART0_TX_BUFFER_SIZE);

	if (index == UART0_u8TxOut) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

bool UART0_bTxBufferEmpty(void) {
	if(UART0_u8TxOut == UART0_u8TxIn) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

bool UART0_bTxFinished(void) {
	if(UART0_u8TxOut == UART0_u8TxIn) {
		return FALSE;
	}

	return UART0_bTxFinishedFlag;
}

bool UART0_bTxByte(uint8_t u8Data) {
	uint8_t index = UART0_u8TxIn;

	// Calculate next pointer position
	GET_NEXT_BUFF_IDX(index, UART0_TX_BUFFER_SIZE);

	// Make sure there is space available in buffer
	if (index == UART0_u8TxOut) {
		if (UART0_TxBuffOvf != NULL)
			UART0_TxBuffOvf();

		return FALSE;
	}

	// Insert data into buffer
	UART0_u8TxBuffer[UART0_u8TxIn] = u8Data;

	// Advance pointer
	UART0_u8TxIn = index;

	// Make sure transmit process is started by enabling interrupt
	BIT_SET_HI(UCSR0B, UDRIE0);

	return TRUE;
}

extern uint8_t UART0_u8TxData(const uint8_t* pu8Data, uint8_t u8NoBytesToSend) {
	uint8_t bytes_buffered = 0;
	uint8_t index;

	while (u8NoBytesToSend) {
		// Calculate next pointer position
		index = UART0_u8TxIn;
		GET_NEXT_BUFF_IDX(index, UART0_TX_BUFFER_SIZE);

		// Make sure there is space available in buffer
		if (index == UART0_u8TxOut) {
			if (UART0_TxBuffOvf != NULL)
				UART0_TxBuffOvf();

			break;
		}

		// Insert data into buffer
		UART0_u8TxBuffer[UART0_u8TxIn] = *pu8Data++;

		// Advance pointer
		UART0_u8TxIn = index;

		// Next byte
		bytes_buffered++;
		u8NoBytesToSend--;
	}

	// Make sure transmit process is started by enabling interrupt
	BIT_SET_HI(UCSR0B, UDRIE0);

	return bytes_buffered;
}

/**
 *  @}
 */
