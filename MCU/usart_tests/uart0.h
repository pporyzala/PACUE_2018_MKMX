#ifndef __UART0_H__
#define __UART0_H__

#include "global.h"

/// BAUD rate of UART0
#define UART0_BAUD 250000ul

typedef enum {
	//! no parity
    UART0_NO_PARITY = 0,
    //! odd parity
    UART0_ODD_PARITY,
	//! even parity
    UART0_EVEN_PARITY
} UART0_Parity_t;

typedef enum {
	//! no error
    UART0_NO_ERROR = 0,
    //! framing error
    UART0_FRAMING_ERROR,
	//! data overrun error
    UART0_DATA_OVERRUN_ERROR,
    //! parity error
    UART0_PARITY_ERROR
} UART0_Error_t;

/* _____PUBLIC FUNCTION DECLARATIONS_________________________________________ */
/**
 *  Initialize UART0 peripheral, buffers and interrupt handlers.
 *
 *  @param baud          Baud rate in bits/s
 *  @param data_bits     Data bits (5,6,7 or 8)
 *  @param parity        Parity(NONE, ODD or EVEN)
 *  @param stop_bits     Stop bits (1 or 2)
 */
void UART0_vInit(uint32_t       baud,
                 uint8_t        data_bits,
                 UART0_Parity_t parity,
                 uint8_t        stop_bits);

/**
 *  Set handler for UART0 Rx Buffer Overflow Error.
 *
 *  @param UART0_RxBuffOvf_func		Handler called in case rx buffer overflow error
 */
void UART0_vSetRxBuffOvfHandler(void (*UART0_RxBuffOvf_func)(void));

/**
 *  Set handler for UART0 Tx Buffer Overflow Error.
 *
 *  @param UART0_TxBuffOvf_func		Handler called in case tx buffer overflow error
 */
void UART0_vSetTxBuffOvfHandler(void (*UART0_TxBuffOvf_func)(void));

/**
 *  Set handler for UART0 Rx Data Error (data overrun / frame / parity error) handler,
 *  handler should be in form of: void xxx(UART0_Error_t eError).
 *
 *  @param UART0_RxDataErr_func		Handler called in case of error
 */
void UART0_vSetRxDataErrHandler(void (*UART0_RxDataErr_func)(UART0_Error_t eError));

/**
 *  Change to new BAUD rate.
 *
 *  Calculates and sets the new 16-bit UBBR register value. No check is
 *  performed that the actual BAUD rate is within tolerance of the specified
 *  BAUD rate.
 *
 *  UBBR = F_OSC / (16 * BAUD) - 1 (for Asynchronous Normal Mode; U2X flag = 0)
 *
 *  @param[in] baud  New BAUD rate
 */
void UART0_vChangeBaud(uint32_t baud);

/**
 *  See if there is received data in the receive buffer.
 *
 *  @retval TRUE    There is received data in the receive buffer
 *  @retval FALSE   The receive buffer is empty
 */
bool UART0_bRxBufferEmpty(void);

/**
 *  See if received byte is available and store it in specified location.
 *
 *  @param[out] 	pu8Data    Pointer to location where data byte must be stored
 *
 *  @retval TRUE        Received byte is stored in specified location
 *  @retval FALSE       No received data available (receive buffer empty)
 */
bool UART0_bGetRxByte(uint8_t* pu8Data);

/**
 *  Copy received data from ring buffer into specified buffer.
 *
 *  @param[out] 	pu8Buffer      			Buffer to copy received data into
 *  @param[in] 		u8MaximumBufferSize 	Maximum number of received bytes to copy into buffer
 *
 *  @return uint8_t         Number of received bytes copied into buffer
 */
uint8_t UART0_u8GetRxData(uint8_t* pu8Buffer, uint8_t u8MaximumBufferSize);

/**
 *  Flush receive buffer.
 */
void UART0_vFlushRxData(void);

/**
 *  Get actual number of bytes in Rx buffer.
 *
 *  @return uint8_t 		Number of bytes in Rx buffer
 */
uint8_t UART0_u8NoOfRxBytes(void);

/**
 *  See if transmit ring buffer can accept more data.
 *
 *  @retval TRUE    Transmit ring buffer is full
 *  @retval FALSE   Transmit ring buffer has space for at least one byte
 */
bool UART0_bTxBufferFull(void);

/**
 *  See if transmit ring buffer is empty.
 *
 *  @note Buffer may be empty, but UART peripheral may still be busy
 *        with the transmission of the last byte in the buffer.
 *        @see uart0_tx_finished.
 *
 *  @retval TRUE    Transmit ring buffer is empty
 *  @retval FALSE   Transmit ring buffer has space for at least one byte
 */
bool UART0_bTxBufferEmpty(void);

/**
 *  Get actual number of bytes in Tx buffer.
 *
 *  @return uint8_t 		Number of bytes in Tx buffer
 */
uint8_t UART0_u8NoOfTxBytes(void);

uint8_t UART0_u8NoOfFreeTxBufferBytes(void);

/**
 *  See if all transmission has finished, including last byte.
 *
 *  This functions is usefull for communication standards like RS-485
 *  where the mode must be changed manually from TX to RX after transmission.
 *
 *  @retval TRUE    Transmision completely finished
 *  @retval FALSE   Busy with transmission
 */
bool UART0_bTxFinished(void);

/**
 *  Buffer one byte for transmission.
 *
 *  @param[in] u8Data   Byte to be transmitted
 *
 *  @retval TRUE    Byte has been buffered
 *  @retval FALSE   Byte has not been buffered, because transmit buffer is full
 */
bool UART0_bTxByte(uint8_t u8Data);

/**
 *  Buffer byte(s) for transmission.
 *
 *  @note The transmit ring buffer may not be able to hold all of the specified
 *        data.
 *
 *  @param[in] pu8Data				Buffer containing data for transmission
 *  @param[in] u8NoBytesToSend		Number of bytes in buffer to be transmitted
 *
 *  @return uint8_t        The actual number of bytes buffered for transmission.
 */
uint8_t UART0_u8TxData(const uint8_t* pu8Data, uint8_t u8NoBytesToSend);

#endif
