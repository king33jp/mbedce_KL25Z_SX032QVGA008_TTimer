#ifndef CARDREADER_H
#define CARDREADER_H

// RCS620S
#define COMMAND_TIMEOUT               400
#define POLLING_INTERVAL              800
// FeliCa Service/System Code
#define CYBERNE_SYSTEM_CODE           0x0003
#define COMMON_SYSTEM_CODE            0xFE00
#define PASSNET_SERVICE_CODE          0x090F
#define EDY_SERVICE_CODE              0x170F
#define NANACO_SERVICE_CODE           0x564F
#define WAON_SERVICE_CODE             0x680B

#define COMPANY_ID_SYSTEM_CODE        0x82FD
#define COMPANY_ID_SERVICE_CODE       0x020B
//#define MAX_MEMORY 21   // <128/6
#define MAX_MEMORY 10   // <128/(6*2)

char *itoh(uint8_t val, char *ptr);
int requestService(uint16_t serviceCode);
int readEncryption(uint16_t serviceCode, uint8_t blockNumber, uint8_t *buf);
void printBalanceLCD(const char *card_name, uint32_t *balance, int non,int detnow);
void printIDCard(uint8_t *buf,int detnow);
int cardreading_mode(int num);

#endif
