#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/delay.h>

#include "protocol.h"

int main(void)
{
	ProtcolInit();
	
	sei();

    /* Replace with your application code */
    while (1) 
    {
		ParseData();
		 //char u8Payload[] = "test ;-)";
		 //SendData(0x99, 't', u8Payload, strlen(u8Payload));
		 //SendText(0x99, "test ;-)");
		 //_delay_ms(500);
    }
}

