#include "mkmx_state_machine.h"
static MkmxMachine_t MkmxMachine;

void MkmxInit(uint8_t address, uint8_t(*_crc)(uint8_t, uint8_t)){
    // all fields are initialised to provide platform for automated testing
    MkmxMachine.state = MKMX_IDLE;
    MkmxMachine.deviceAddress = address;
    MkmxMachine.command = 0;
    MkmxMachine.payloadLength = 0;
    MkmxMachine.payloadPosition = 0;
    MkmxMachine.crc_received = 0;
    MkmxMachine.crcFunction = _crc;
    MkmxMachine.isFrameReady = 0;

    for(uint8_t i=0; i<MKMX_MAX_INPUT_PAYLOAD_SIZE; ++i){
        MkmxMachine.payload[i]=0;
    }
}
MkmxState_t MkmxUpdate(uint8_t _rx){
    uint8_t checksum;
    switch(MkmxMachine.state){
        case MKMX_IDLE:
            if(_rx == 0x5A)     MkmxMachine.state = MKMX_SOF1;
            else                MkmxMachine.state = MKMX_IDLE;
            break;
        case MKMX_SOF1:
            if(_rx == 0xA5)     MkmxMachine.state = MKMX_SOF2;
            else                MkmxMachine.state = MKMX_IDLE;
            break;
        case MKMX_SOF2:
            if(_rx == MkmxMachine.deviceAddress)  MkmxMachine.state = MKMX_ADDR;
            else                MkmxMachine.state = MKMX_XADDR;
            break;
        case MKMX_ADDR:
            MkmxMachine.command = _rx;
                                MkmxMachine.state = MKMX_CMD;
            break;
        case MKMX_CMD:
            MkmxMachine.payloadPosition = 0;
            MkmxMachine.payloadLength = _rx;
            if(MkmxMachine.payloadLength == 0)
                                MkmxMachine.state = MKMX_PLCPL;
            if(MkmxMachine.payloadLength > MKMX_MAX_INPUT_PAYLOAD_SIZE)
                MkmxMachine.payloadLength = MKMX_MAX_INPUT_PAYLOAD_SIZE;
                                MkmxMachine.state = MKMX_PLRX;
            break;
        case MKMX_PLRX:
            MkmxMachine.payload[MkmxMachine.payloadPosition++] = _rx;
            if(MkmxMachine.payloadPosition >= MkmxMachine.payloadLength)
                                MkmxMachine.state = MKMX_PLCPL;
            break;
        case MKMX_PLCPL:
            // calculate checksum
            checksum = 0;
            checksum = MkmxMachine.crcFunction(checksum, MkmxMachine.deviceAddress);
            checksum = MkmxMachine.crcFunction(checksum, MkmxMachine.command);
            checksum = MkmxMachine.crcFunction(checksum, MkmxMachine.payloadLength);
            for(uint8_t i=0; i<MkmxMachine.payloadLength; ++i){
                checksum = MkmxMachine.crcFunction(checksum, MkmxMachine.payload[i]);
            }
            if(checksum == _rx && MkmxMachine.isFrameReady == 0){
                // checksum valid, buffer empty
                MkmxFrame.command = MkmxMachine.command;
                MkmxFrame.payloadLength = MkmxMachine.payloadLength;
                for(uint8_t i=0; i<MkmxMachine.payloadLength; ++i){
                    MkmxFrame.payload[i] = MkmxMachine.payload[i];
                }
                MkmxMachine.isFrameReady = 1;
            }
                                MkmxMachine.state = MKMX_IDLE;
            break;
        case MKMX_XADDR:
                                MkmxMachine.state = MKMX_XCMD;
            break;
        case MKMX_XCMD:
            MkmxMachine.payloadPosition = 0;
            MkmxMachine.payloadLength = _rx;
            if(MkmxMachine.payloadLength == 0)
                                MkmxMachine.state = MKMX_XPLCPL;
            else                MkmxMachine.state = MKMX_XPLRX;
            break;
        case MKMX_XPLRX:
            ++MkmxMachine.payloadPosition;
            if(MkmxMachine.payloadPosition >= MkmxMachine.payloadLength)
                                MkmxMachine.state = MKMX_XPLCPL;
            break;
        case MKMX_XPLCPL:
                                MkmxMachine.state = MKMX_IDLE;
            break;
        default:
            // this place is nver reached
            break;
    }
    return MkmxMachine.state;
}
uint8_t MkmxIsReady(void){
    return MkmxMachine.isFrameReady;
}
void MkmxDiscardFrame(void){
    MkmxMachine.isFrameReady = 0;
}
