#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <stdint.h>

// Standard types
typedef uint8_t bool;

#ifndef CRITICAL_SECTION_START
	#define CRITICAL_SECTION_START	unsigned char _sreg = SREG; cli()
	#define CRITICAL_SECTION_END	SREG = _sreg
#endif

// Boolean types
#define FALSE ((bool)0)
#define TRUE  ((bool)1)

#define false ((bool)0)
#define true  ((bool)1)

// NULL pointer
#ifndef NULL
	#define NULL (void*)0
#endif

// Bit macros
#define BIT_SET_HI(Port,Bit)             { Port |= (unsigned char)(1 << Bit); }
#define BIT_SET_LO(Port,Bit)             { Port &= (unsigned char)~(1 << Bit); }
#define BIT_TOGGLE(Port,Bit)             {if(Port&(1<<Bit)) {Port &= (unsigned char)~(1<<Bit);} else {Port |=(unsigned char)(1<<Bit);}}
#define BIT_IS_HI(Port,Bit)              ((Port&(unsigned char)(1<<Bit)) != 0)
#define BIT_IS_LO(Port,Bit)              ((Port&(unsigned char)(1<<Bit)) == 0)
#define LOOP_UNTIL_BIT_IS_HI(Port,Bit)   while(BIT_IS_LO(Port,Bit)) {;}
#define LOOP_UNTIL_BIT_IS_LO(Port,Bit)   while(BIT_IS_HI(Port,Bit)) {;}

// Byte macros
#define U16_HI(u16Data) ((uint8_t)((u16Data>>8)&0xff))
#define U16_LO(u16Data) ((uint8_t)(u16Data&0xff))

#define U32_HH(u32Data) ((uint8_t)((u32Data>>24)&0xff))
#define U32_HL(u32Data) ((uint8_t)((u32Data>>16)&0xff))
#define U32_LH(u32Data) ((uint8_t)((u32Data>>8)&0xff))
#define U32_LL(u32Data) ((uint8_t)(u32Data&0xff))

#define GET_NEXT_BUFF_IDX(u8BufferIdx, BUFFER_SIZE) \
        { \
            uint8_t u8Temp = u8BufferIdx; \
            u8Temp++; \
            /* See if buffer size is a power of two */ \
            if((BUFFER_SIZE&(BUFFER_SIZE-1)) == 0) \
            { \
                /* Use masking to optimize pointer wrapping to index 0 */ \
                u8Temp &= (BUFFER_SIZE-1); \
            } \
            else \
            { \
                /* Wrap index to 0 if it has exceeded buffer boundary */ \
                if(u8Temp == (uint8_t)BUFFER_SIZE) u8Temp = 0; \
            } \
            u8BufferIdx = u8Temp; \
        }

#define GET_PREVIOUS_BUFF_IDX(u8BufferIdx, BUFFER_SIZE) \
        { \
            uint8_t u8Temp = u8BufferIdx; \
            u8Temp--; \
            /* See if buffer size is a power of two */ \
            if((BUFFER_SIZE&(BUFFER_SIZE-1)) == 0) \
            { \
                /* Use masking to optimize pointer wrapping to index 0 */ \
                u8Temp &= (BUFFER_SIZE-1); \
            } \
            else \
            { \
                /* Wrap index to 0 if it has exceeded buffer boundary */ \
                if(u8BufferIdx == (uint8_t)0) u8Temp = (BUFFER_SIZE-1); \
            } \
            u8BufferIdx = u8Temp; \
        }

//! Computes the minimum of a and b.
#define MIN(a,b)			((a<b)?(a):(b))
//! Computes the maximum of a and b.
#define MAX(a,b)			((a>b)?(a):(b))
//! Computes the absolute value of its argument x.
#define ABS(x)				((x>0)?(x):(-x))

#define NOP() {asm volatile("nop"::);}

// Constant divide calculation with rounding
#define DIV(Dividend,Divisor) (((Dividend+((Divisor)>>1))/(Divisor)))
/**
 *  @}
 */

#endif
