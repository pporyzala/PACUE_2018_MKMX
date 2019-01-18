#ifndef CRCTOOLS_H
#define CRCTOOLS_H

#include <stdint.h>

uint8_t _crc8_ccitt_update(uint8_t inCrc, uint8_t inData);
uint8_t computeCRC(uint8_t* pu8Data, uint16_t u16Len);

#endif // CRCTOOLS_H

