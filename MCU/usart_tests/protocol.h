#ifndef PROTCOL_H_
#define PROTCOL_H_

void ProtcolInit(void);

void ParseData(void);

void SendData(uint8_t u8Addr, uint8_t u8Cmd, uint8_t *pu8Payload, uint8_t u8PayloadLen);
void SendText(uint8_t u8Addr, char *pcStr);

#endif /* PROTCOL_H_ */