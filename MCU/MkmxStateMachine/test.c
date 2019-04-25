#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "mkmx_state_machine.h"

uint8_t _crc8_ccitt_update (uint8_t inCrc, uint8_t inData)
{
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
void Test_begin(void){
    MkmxInit(0x42, _crc8_ccitt_update);
    MkmxFrame.command = 0;
    MkmxFrame.payloadLength = 0;
    for(uint8_t i=0; i<MKMX_MAX_INPUT_PAYLOAD_SIZE; ++i){
        MkmxFrame.payload[i] = 0;
    }
}
int main()
{
    // TEST: waiting for SOF
    Test_begin();
    for(uint16_t i=0; i<256; ++i){
        if(i != 0x5A) assert(MkmxUpdate((uint8_t)i) == MKMX_IDLE);
    }


    // TEST: SOF1
    Test_begin();
    assert(MkmxUpdate(0x5A) == MKMX_SOF1);
    assert(MkmxUpdate(0xA4) == MKMX_IDLE);


    // TEST: SOF1 + SOF2
    Test_begin();
    assert(MkmxUpdate(0x5A) == MKMX_SOF1);
    assert(MkmxUpdate(0xA5) == MKMX_SOF2);


    // TEST: Transaction
    Test_begin();
    assert(MkmxUpdate(0x5A) == MKMX_SOF1);
    assert(MkmxUpdate(0xA5) == MKMX_SOF2);
    assert(MkmxUpdate(0x42) == MKMX_ADDR);
    assert(MkmxUpdate(0x99) == MKMX_CMD);
    assert(MkmxUpdate(0x01) == MKMX_PLRX);
    assert(MkmxUpdate(0xDE) == MKMX_PLCPL);
    assert(MkmxUpdate(0x25) == MKMX_IDLE);
    assert(MkmxIsReady());
    assert(MkmxFrame.command == 0x99);
    assert(MkmxFrame.payloadLength == 0x01);
    assert(MkmxFrame.payload[0] == 0xDE);


    // TEST: Invalid CRC
    Test_begin();
    assert(MkmxUpdate(0x5A) == MKMX_SOF1);
    assert(MkmxUpdate(0xA5) == MKMX_SOF2);
    assert(MkmxUpdate(0x42) == MKMX_ADDR);
    assert(MkmxUpdate(0x99) == MKMX_CMD);
    assert(MkmxUpdate(0x01) == MKMX_PLRX);
    assert(MkmxUpdate(0xDE) == MKMX_PLCPL);
    assert(MkmxUpdate(0x24) == MKMX_IDLE);
    assert(!MkmxIsReady());


    // TEST: not my address
    Test_begin();
    assert(MkmxUpdate(0x5A) == MKMX_SOF1);
    assert(MkmxUpdate(0xA5) == MKMX_SOF2);
    assert(MkmxUpdate(0x24) == MKMX_XADDR);
    assert(MkmxUpdate(0x99) == MKMX_XCMD);
    assert(MkmxUpdate(0x01) == MKMX_XPLRX);
    assert(MkmxUpdate(0x00) == MKMX_XPLCPL);
    assert(MkmxUpdate(0x13) == MKMX_IDLE);
    assert(!MkmxIsReady());


    // TEST: not my address, huge frame
    Test_begin();
    assert(MkmxUpdate(0x5A) == MKMX_SOF1);
    assert(MkmxUpdate(0xA5) == MKMX_SOF2);
    assert(MkmxUpdate(0x24) == MKMX_XADDR);
    assert(MkmxUpdate(0x99) == MKMX_XCMD);
    assert(MkmxUpdate(0xFF) == MKMX_XPLRX);
    for(uint i=0; i<254; ++i){
        assert(MkmxUpdate(0x00) == MKMX_XPLRX);
    }
    assert(MkmxUpdate(0x00) == MKMX_XPLCPL);
    assert(MkmxUpdate(0x13) == MKMX_IDLE);
    assert(!MkmxIsReady());


    // TEST: frame ready
    Test_begin();
    MkmxUpdate(0x5A);
    MkmxUpdate(0xA5);
    MkmxUpdate(0x42);
    MkmxUpdate(0x99);
    MkmxUpdate(0x01);
    MkmxUpdate(0xDE);
    MkmxUpdate(0x25);
    assert(MkmxIsReady());
    MkmxDiscardFrame();
    assert(!MkmxIsReady());


    printf("All tests passed\n");
    return 0;
}