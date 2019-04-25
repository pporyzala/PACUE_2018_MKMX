#ifndef MKMXSTATEMACHINE_H_INCLUDED
#define MKMXSTATEMACHINE_H_INCLUDED

#include <stdint.h>

// maximum size of payload hat can be accepted by the state machine
// applicable for *this address* only, other payloads can be 255 bytes long
#define MKMX_MAX_INPUT_PAYLOAD_SIZE     32

// possible states
typedef enum {  MKMX_IDLE,   // wait for SOF1
                MKMX_SOF1,   // acquired SOF1 = 0x5A
                MKMX_SOF2,   // acq  SOF2 = 0xA5
                MKMX_ADDR,   // acq address
                    MKMX_XADDR, // disposing unwanted frames
                    MKMX_XCMD,
                    MKMX_XPLRX,
                    MKMX_XPLCPL,
                MKMX_CMD,    // acq cmd
                MKMX_PLRX,   // payload acquisition in progress
                MKMX_PLCPL,  // payload acquisition complete, awaiting crc
                } MkmxState_t;

// internal data exchange of the state machine
typedef struct {
    MkmxState_t state;
    uint8_t deviceAddress;
    uint8_t command;
    uint8_t payloadLength;
    uint8_t payloadPosition;
    uint8_t payload[MKMX_MAX_INPUT_PAYLOAD_SIZE];
    uint8_t crc_received;
    uint8_t (*crcFunction)(uint8_t, uint8_t);
    uint8_t isFrameReady;

} MkmxMachine_t;

// communication with user
struct {
    uint8_t command;
    uint8_t payloadLength;
    uint8_t payload[MKMX_MAX_INPUT_PAYLOAD_SIZE];
} MkmxFrame;

void MkmxInit(uint8_t address, uint8_t(*_crc)(uint8_t, uint8_t));
MkmxState_t MkmxUpdate(uint8_t _rx);
uint8_t MkmxIsReady(void);
void MkmxDiscardFrame(void);
#endif // MKMXSTATEMACHINE_H_INCLUDED
