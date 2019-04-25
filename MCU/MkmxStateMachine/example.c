// przyklad uzycia
#include <util/crc16.h>
#include "uart.h"
#include "mkmx_state_machine.h"

int main(void){
	
	// inicjalizuje maszyne stanow na odbior ramek o adresie 0x42
    MkmxInit(0x42, _crc8_ccitt_update);

    while(1){
		// pobiera bajt
        uint16_t tmp = uart_getc();
		
		// sprawdza bajt i wrzuca go do maszyny
        if ((tmp & 0xFF00) == 0) MkmxUpdate((uint8_t) tmp);
		
		// sprawdza czy odebrano nowa ramke
        if(MkmxIsReady()){
			
			/* TUTAJ WYKONUJESZ OPERACJE NA DANYCH Z RAMKI*/
			// dane z ramki sa w strukturze MkmxFrame
			
			
            // potwierdza gotowosc do odbioru nowych ramek
			// bez wywolania tej funkcji nowe ramki nie beda odbierane
            MkmxDiscardFrame();
        }
        // dalszy ciag glownej petli
    }
}