#include "ringbuffer.h"

static inline uint16_t u16GetNextBuffIdx(uint16_t u16BufferIdx, uint16_t u16BUFFER_SIZE) {
    uint16_t u16Temp = u16BufferIdx;
    u16Temp++;

    // See if buffer size is a power of two
    if ((u16BUFFER_SIZE & (u16BUFFER_SIZE - 1)) == 0) {
        // Use masking to optimize pointer wrapping to index 0
        u16Temp &= (u16BUFFER_SIZE - 1);
    } else {
        // Wrap index to 0 if it has exceeded buffer boundary
        if (u16Temp == u16BUFFER_SIZE) u16Temp = 0;
    }

    return u16Temp;
}

void InitializeRingBuffer(sRingBuffer_t *psBuffer, uint8_t *pu8Buffer, uint16_t u16BuffSize, void (*vBuffOvf_func)(void)) {
    psBuffer->u16In = 0;
    psBuffer->u16Out = 0;

    psBuffer->pu8Buffer = pu8Buffer;
    psBuffer->u16Size = u16BuffSize;

    psBuffer->vOvfHandler = vBuffOvf_func;
}

void Flush(sRingBuffer_t *psBuffer) {
    psBuffer->u16In = psBuffer->u16Out = 0;
}

uint16_t NoOfUsedBytes(sRingBuffer_t *psBuffer) {
    if (psBuffer->u16In >= psBuffer->u16Out)
        return (psBuffer->u16In - psBuffer->u16Out);
    else
        return ((psBuffer->u16Size - psBuffer->u16Out) + psBuffer->u16In);
}

uint16_t NoOfFreeBytes(sRingBuffer_t *psBuffer) {
    return psBuffer->u16Size - NoOfUsedBytes(psBuffer) - 1;
}

bool IsFull(sRingBuffer_t *psBuffer) {
    // Calculate next pointer position
    uint16_t index = u16GetNextBuffIdx(psBuffer->u16In, psBuffer->u16Size);

    if (index == psBuffer->u16Out)
        return true;
    else
        return false;
}

bool IsEmpty(sRingBuffer_t *psBuffer) {
    if (psBuffer->u16Out == psBuffer->u16In)
        return true;
    else
        return false;
}

bool PushByte(sRingBuffer_t *psBuffer, uint8_t u8Data) {
    // Calculate next pointer position
    uint16_t index = u16GetNextBuffIdx(psBuffer->u16In, psBuffer->u16Size);

    // Make sure there is space available in buffer
    if (index == psBuffer->u16Out) {
        if (psBuffer->vOvfHandler != 0)
            psBuffer->vOvfHandler();

        return false;
    }

    // Insert data into buffer
    psBuffer->pu8Buffer[psBuffer->u16In] = u8Data;

    // Advance pointer
    psBuffer->u16In = index;

    return true;
}

uint8_t PushData(sRingBuffer_t *psBuffer, const uint8_t* pu8Data, uint8_t u8NoBytesToSend) {
    uint8_t bytes_buffered = 0;

    while (u8NoBytesToSend) {
        if (PushByte(psBuffer, *pu8Data)) {
            pu8Data++;

            // Next byte
            bytes_buffered++;
            u8NoBytesToSend--;
        } else {
            break;
        }
    }

    return bytes_buffered;
}

bool PopByte(sRingBuffer_t *psBuffer, uint8_t* pu8Data) {
    if (IsEmpty(psBuffer))
        return false;

    *pu8Data = psBuffer->pu8Buffer[psBuffer->u16Out];

    // Advance pointer
    psBuffer->u16Out = u16GetNextBuffIdx(psBuffer->u16Out, psBuffer->u16Size);

    return true;
}

uint8_t PopData(sRingBuffer_t *psBuffer, uint8_t* pu8DataBuffer, uint8_t u8MaxDataBytes) {
    uint8_t u8NoOfPoppedBytes = 0;

    while (u8NoOfPoppedBytes < u8MaxDataBytes) {
        if (IsEmpty(psBuffer))
            return u8NoOfPoppedBytes;

        *(pu8DataBuffer + u8NoOfPoppedBytes) = psBuffer->pu8Buffer[psBuffer->u16Out];
        u8NoOfPoppedBytes++;

        // Advance pointer
        psBuffer->u16Out = u16GetNextBuffIdx(psBuffer->u16Out, psBuffer->u16Size);
    }

    return u8NoOfPoppedBytes;
}
