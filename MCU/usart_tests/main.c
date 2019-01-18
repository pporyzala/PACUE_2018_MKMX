#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/crc16.h>
#include <util/delay.h>

#include "uart0.h"

#define MAX_PAYLOAD_LENGTH	128

//typedef struct {
	//uint16_t u16Start;
	//uint8_t u8DestAddr;
	//uint8_t u8Cmd;
	//uint8_t u8Len;
	//uint8_t u8Payload[MAX_PAYLOAD_LENGTH];
	//uint8_t u8CRC;
//
	//uint8_t u8RawData[MAX_PAYLOAD_LENGTH + 3];
//} sRxFrame_t;

uint8_t checkcrc(uint8_t *pu8Data, uint8_t u8Len) {
	uint8_t crc = 0, i;
	
	for (i = 0; i < u8Len; i++)
		crc = _crc8_ccitt_update(crc, pu8Data[i]);
	
	return crc;
}

void SendDebugData(char* str) {
	uint8_t u8Buff[MAX_PAYLOAD_LENGTH + 6];

	//header:
	u8Buff[0] = 0x5A;
	u8Buff[1] = 0xA5;

	//addr
	u8Buff[2] = 0x00;
	//cmd
	u8Buff[3] = 't';

	//payload
	uint8_t u8PayloadLen = 0;
	while (*str) {
		u8Buff[5 + u8PayloadLen] = *str;
		u8PayloadLen++;
		str++;
	}

	//payload len
	u8Buff[4] = u8PayloadLen;
	//crc
	u8Buff[2 + 3 + u8PayloadLen] = checkcrc(&u8Buff[2], 3 + u8PayloadLen);

	UART0_u8TxData(u8Buff, 2 + 3 + u8PayloadLen + 1);

}

uint8_t u8Buff[255 + 6] = { 0x5A, 0xA5, 0x00, 0x74, 0x04,
							0x31, 0x32, 0x33, 0x34, 0xFF};
uint8_t u8Len = 10;

void SendData(uint8_t *pu8Data, uint16_t u16Len) {
}

int main(void)
{
	UART0_vInit(4800, 8, UART0_NO_PARITY, 1);
	
	sei();

    /* Replace with your application code */
    while (1) 
    {
		//UART0_bTxByte('a');
		SendDebugData("1234");
		_delay_ms(500);
    }
}

