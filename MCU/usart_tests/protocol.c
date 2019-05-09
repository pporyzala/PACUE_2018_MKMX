#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/crc16.h>

#include "uart.h"

#define OWN_ADDRESS		0x55


static uint8_t CalcCRC(uint8_t *pu8Data, uint16_t u16Len);
static void ParseFrame(void);

enum {
	eIdle = 0,
	eWaitSOF,
	eAddress,
	eCmd,
	ePayloadLen,
	ePayload,
	eCRC
} eProtocolState;

struct {
	uint8_t u8Addr;
	uint8_t u8Cmd;
	uint8_t u8PayloadLen;
	uint8_t u8Payload[255];
	uint8_t u8CRC;

	uint8_t u8RawData[3 + 255];
} sFrame;

void ProtcolInit(void) {
	uart_init(UART_BAUD_SELECT(4800, 16000000ul));
}

void ParseData(void) {
	uint16_t u16Word = uart_getc();
	if ((u16Word & 0xFF00) == 0) {
		
		uint8_t u8Byte = u16Word & 0x00FF;
		static uint8_t u8PayloadIdx;
		static uint8_t u8PayloadLen;
		
		uart_putc(u8Byte);

		//uart_puts("state przed: ");
		//uart_putc(eProtocolState + '0');
		//uart_puts("\n");

		switch(eProtocolState) {
			case eIdle:
			if (u8Byte == 0x5A) {
				eProtocolState = eWaitSOF;
			}
			break;

			case eWaitSOF:
			if (u8Byte == 0xA5) {
				eProtocolState = eAddress;
				} else {
				eProtocolState = eIdle;
			}
			break;

			case eAddress:
			sFrame.u8Addr = u8Byte;
			sFrame.u8RawData[0] = u8Byte;
			eProtocolState = eCmd;
			break;

			case eCmd:
			sFrame.u8Cmd = u8Byte;
			sFrame.u8RawData[1] = u8Byte;
			eProtocolState = ePayloadLen;
			break;

			case ePayloadLen:
			sFrame.u8PayloadLen = u8Byte;
			sFrame.u8RawData[2] = u8Byte;

			if (u8Byte != 0) {
				eProtocolState = ePayload;

				u8PayloadIdx = 0;
				u8PayloadLen = u8Byte;
				} else {
				eProtocolState = eCRC;
			}
			break;

			case ePayload:
			sFrame.u8Payload[u8PayloadIdx] = u8Byte;
			sFrame.u8RawData[3 + u8PayloadIdx] = u8Byte;
			u8PayloadIdx++;
			
			if (u8PayloadIdx == sFrame.u8PayloadLen) {
				eProtocolState = eCRC;
			}
			break;

			case eCRC:
			sFrame.u8CRC = u8Byte;
			uint8_t u8CalcCRC =
			CalcCRC((uint8_t *)&sFrame, 3 + sFrame.u8PayloadLen);
			
			if (u8CalcCRC == sFrame.u8CRC) {
				ParseFrame();
			} else {
				uart_puts("zostajemy!! :-(");
			}

			eProtocolState = eIdle;
			break;
		}

		uart_puts("  state po: ");
		uart_putc(eProtocolState + '0');
		uart_puts("\n");
	}
}

void ParseFrame(void) {
	if (sFrame.u8Addr == OWN_ADDRESS) {
		switch(sFrame.u8Cmd) {
			case 0x00:
		
			break;
		
			case 0x01:
		
			break;
		
			default:
		
			break;
		}
	}
}

void SendData(uint8_t u8Addr, uint8_t u8Cmd,
			uint8_t *pu8Payload, uint8_t u8PayloadLen) {
	uint8_t u8Buff[255 + 6];
	u8Buff[0] = 0x5A;
	u8Buff[1] = 0xA5;
	u8Buff[2] = u8Addr;
	u8Buff[3] = u8Cmd;
	u8Buff[4] = u8PayloadLen;
	for (uint8_t i = 0; i < u8PayloadLen; i++) {
		u8Buff[5 + i] = pu8Payload[i];
	}
	u8Buff[5 + u8PayloadLen] = CalcCRC(&u8Buff[2], 3 + u8PayloadLen);

	uart_putdata(u8Buff, 5 + u8PayloadLen + 1);
}

void SendText(uint8_t u8Addr, char *pcStr) {
	uint8_t u8Buff[255 + 6];
	u8Buff[0] = 0x5A;
	u8Buff[1] = 0xA5;
	u8Buff[2] = u8Addr;
	u8Buff[3] = 't';

	uint8_t u8PayloadLen = 0;
	while (*pcStr != 0) {
		u8Buff[5 + u8PayloadLen] = *pcStr;
		pcStr++;
		u8PayloadLen++;
	}

	u8Buff[4] = u8PayloadLen;

	u8Buff[5 + u8PayloadLen] = CalcCRC(&u8Buff[2], 3 + u8PayloadLen);

	uart_putdata(u8Buff, 5 + u8PayloadLen + 1);
}

static uint8_t CalcCRC(uint8_t *pu8Data, uint16_t u16Len) {
	uint8_t u8CRC = 0;
	for (uint16_t i = 0; i < u16Len; i++) {
		u8CRC = _crc8_ccitt_update(u8CRC, pu8Data[i]);
	}

	return u8CRC;
}