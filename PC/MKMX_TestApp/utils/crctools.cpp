#include "crctools.h"

#include "debugtools.h"

uint8_t _crc8_ccitt_update(uint8_t inCrc, uint8_t inData) {
    uint8_t   i;
    uint8_t   data;
    data = inCrc ^ inData;

    for ( i = 0; i < 8; i++ ) {
        if (( data & 0x80 ) != 0 ) {
            data <<= 1;
            data ^= 0x07;
        } else {
            data <<= 1;
        }
    }
    return data;
}

uint8_t computeCRC(uint8_t* pu8Data, uint16_t u16Len) {
    uint8_t crc = 0;
    uint16_t i;

    for (i = 0; i < u16Len; i++) {
        qDebug() << u8ToString(pu8Data[i]);
        crc = _crc8_ccitt_update(crc, pu8Data[i]);
    }

    return crc;
}
