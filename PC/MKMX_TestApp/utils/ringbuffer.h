#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__

#include <stdint.h>

typedef struct {
    /// Transmit ring (circular) buffer
    uint8_t *pu8Buffer;

    uint16_t u16Size;

    //in & out pointers
    volatile uint16_t u16Out;
    volatile uint16_t u16In;

    //overflow handler
    void (* vOvfHandler)(void);
} sRingBuffer_t;

void InitializeRingBuffer(sRingBuffer_t *psBuffer, uint8_t *pu8Buffer, uint16_t u16BuffSize, void (*vBuffOvf_func)(void));

void Flush(sRingBuffer_t *psBuffer);

uint16_t NoOfUsedBytes(sRingBuffer_t *psBuffer);
uint16_t NoOfFreeBytes(sRingBuffer_t *psBuffer);

bool IsFull(sRingBuffer_t *psBuffer);
bool IsEmpty(sRingBuffer_t *psBuffer);

bool PushByte(sRingBuffer_t *psBuffer, uint8_t u8Data);
uint8_t PushData(sRingBuffer_t *psBuffer, const uint8_t* pu8Data, uint8_t u8NoBytesToSend);

bool PopByte(sRingBuffer_t *psBuffer, uint8_t* pu8Data);
uint8_t PopData(sRingBuffer_t *psBuffer, uint8_t* pu8DataBuffer, uint8_t u8MaxDataBytes);

#endif //__RINGBUFFER_H__
