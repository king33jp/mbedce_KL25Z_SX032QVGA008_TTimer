#include "mbed.h"
#include "cardreader.h"
#include "SX032QVGA008.h"
#include "string"
#include "Consolas10.h"
//#include "Consolas12.h"
#include "Prototype29x28.h"
#include "Prototype24x34_T.h"
#include "Prototype20x25_T.h"
#include "Prototype33x38.h"
//#include "MSGothic12_16x16.h"

#include "RCS620S.h"
#include "oto.h"

extern RCS620S felica;   // RCS620S(PinName p_tx, PinName p_rx)
extern SX032QVGA008 TFT;   // mosi, miso, sclk, cs, rs, reset

extern uint32_t balance;
extern uint8_t buf[RCS620S_MAX_CARD_RESPONSE_LEN];
extern uint32_t namecode_data[MAX_MEMORY];      // 50人分の氏名コード 0をデフォルト表示にする
                            // 氏名コードを4byteの整数で覚える

// request service
int requestService(uint16_t serviceCode){
  int ret;
  uint8_t buf[RCS620S_MAX_CARD_RESPONSE_LEN];
  uint8_t responseLen = 0;
   
  buf[0] = 0x02;
  memcpy(buf + 1, felica.idm, 8);
  buf[9] = 0x01;
  buf[10] = (uint8_t)((serviceCode >> 0) & 0xff);
  buf[11] = (uint8_t)((serviceCode >> 8) & 0xff);
 
  ret = felica.cardCommand(buf, 12, buf, &responseLen);
   
  if(!ret || (responseLen != 12) || (buf[0] != 0x03) ||
      (memcmp(buf + 1, felica.idm, 8) != 0) || ((buf[10] == 0xff) && (buf[11] == 0xff))) {
    return 0;
  }
 
  return 1;
}
 
int readEncryption(uint16_t serviceCode, uint8_t blockNumber, uint8_t *buf){
  int ret;
  uint8_t responseLen = 0;
   
  buf[0] = 0x06;
  memcpy(buf + 1, felica.idm, 8);
  buf[9] = 0x01; // サービス数
  buf[10] = (uint8_t)((serviceCode >> 0) & 0xff);
  buf[11] = (uint8_t)((serviceCode >> 8) & 0xff);
  buf[12] = 0x01; // ブロック数
  buf[13] = 0x80;
  buf[14] = blockNumber;
 
  ret = felica.cardCommand(buf, 15, buf, &responseLen);
 
  if (!ret || (responseLen != 28) || (buf[0] != 0x07) ||
      (memcmp(buf + 1, felica.idm, 8) != 0)) {
    return 0;
  }
 
  return 1;
}
 
//void printBalanceLCD(char *card_name, uint32_t *balance, int non,int detnow){
void printBalanceLCD(const char *card_name, uint32_t *balance, int non,int detnow){
    int pos_x = 50, pos_y = 60;
    if(detnow==0)Buzzer_pipi(1);
    TFT.foreground( GreenYellow );
    TFT.set_font( (unsigned char *)Prototype29x28 );
        // get the center of the screen
        TFT.locate(pos_x+10,pos_y+10);
        TFT.printf("%s",card_name);
        TFT.locate(pos_x+10,pos_y+60);
        TFT.foreground( White );
        TFT.set_font( (unsigned char *)Prototype33x38 );
        //TFT._putc(142); // 残
        TFT._putc(67); // 残
        TFT.foreground(Orange);
        TFT.set_font( (unsigned char *)Prototype29x28 );
        TFT.locate(pos_x+50,pos_y + 60 + 3);
        TFT.printf("%5d",(int)(*balance));
        TFT.foreground( White );
        TFT.set_font( (unsigned char *)Prototype33x38 );
        TFT.locate(pos_x+180,pos_y+60);
        //TFT._putc(145); // 円
        TFT._putc(70); // 円
    return;
}

void printIDCard(uint8_t *buf,int detnow){
    int pos_x = 50, pos_y = 60;
    //TFT.cls();
    if(detnow==0)Buzzer_pipi(2);
        TFT.foreground( Blue );
        TFT.set_font( (unsigned char *)Prototype33x38 );
        TFT.locate(pos_x+10,pos_y+10);
        //TFT.printf("ID Card"); 
        TFT.printf("-,*+/20"); // I(45) D(44) :(42) C(43) a(47) r(50) d(48) 
        TFT.set_font( (unsigned char *)Prototype29x28 );
        TFT.locate(pos_x+30,pos_y+55);
        TFT.foreground( Orange );
        for(int i=15;i<22;i++){
            TFT.printf("%c",buf[i]);
        }
    TFT.set_font( (unsigned char *)Consolas7x13 );
//    TFT.locate(0,45);
//    TFT.foreground(LightGrey);
//    for(int i=12;i<15;i++) TFT.printf("%c",buf[i]); // 上位3byte
//    TFT.foreground(Maroon);
//    TFT.printf(",");
//    TFT.foreground(LightGrey);
//    for(int i=22;i<28;i++) TFT.printf("%c",buf[i]); // 下位6byte
    
    TFT.set_font( (unsigned char *)Consolas7x13 );
    TFT.foreground(White);
    TFT.locate(pos_x+10,pos_y + 90);
    TFT.printf("idm:");
    for (int i=0;i<=7;i++) TFT.printf("%02X",felica.idm[i]);
    TFT.locate(pos_x+10,pos_y + 102);
    TFT.printf("pmm:");
    for (int i=0;i<=7;i++) TFT.printf("%02X",felica.pmm[i]);
}

/** カードリードモード
 * countdown=0はカウントダウンしない。
 * 0以上はカードがない状態にカウントダウンして抜ける
*/
int cardreading_mode(int countdown){
    int flag_detectnow=0;     // 検出中はブザーならさい
    int ret=0;                 // 戻り値、IDカードのときのみIDを返す、それ以外は0
    TFT.background(Black);
    TFT.foreground(White);
    //TFT.cls();
    TFT.locate(0,0);
    TFT.foreground(Green);
    TFT.set_font( (unsigned char*)Consolas7x13 );
    //Buzzer_pipi(3);
    int countnum = countdown;
//    while(1){
//      if(countdown>0){
//          countnum--;
//          if(countnum<0)break;
//      }
      // サイバネ領域
      if(felica.polling( 0xFFFF )){ 
        if(felica.polling(CYBERNE_SYSTEM_CODE)){// Suica PASMO
            if(requestService(PASSNET_SERVICE_CODE)){
                if(readEncryption(PASSNET_SERVICE_CODE, 0, buf)){
                // Little Endianで入っているPASSNETの残高を取り出す
                    balance = (buf[23] << 8) + buf[22]; // 11,10 byte目
                // 残高表示
                    if(flag_detectnow!=1)printBalanceLCD("PASSNET", &balance,1,flag_detectnow);
                    flag_detectnow=1;
                }
            }
        }
        // 共通領域
        else if(felica.polling(COMMON_SYSTEM_CODE)){
            // Edy
            if(requestService(EDY_SERVICE_CODE)){
                if(readEncryption(EDY_SERVICE_CODE, 0, buf)){
             // Big Endianで入っているEdyの残高を取り出す
                 balance = (buf[26] << 8) + buf[27]; // 14,15 byte目
             // 残高表示
                 if(flag_detectnow!=1)printBalanceLCD("Edy", &balance,1,flag_detectnow);
                 flag_detectnow=1;
                }
            }
            // nanaco
            if(requestService(NANACO_SERVICE_CODE)){
                if(readEncryption(NANACO_SERVICE_CODE, 0, buf)){
             // Big Endianで入っているNanacoの残高を取り出す
                    balance = (buf[17] << 24)+(buf[18] << 16)+(buf[19] << 8)+buf[20];   // 5,6,7,8byte目
             // 残高表示
                    if(flag_detectnow!=1)printBalanceLCD("nanaco", &balance,1,flag_detectnow);
                    flag_detectnow=1;
                }
            }
            // waon
            if(requestService(WAON_SERVICE_CODE)){
                if(readEncryption(WAON_SERVICE_CODE, 1, buf)){
                // Big Endianで入っているWaonの残高を取り出す
                    balance = (buf[17]<<16)+(buf[18]<<8)+buf[19];   //21,22,23
                    balance = balance & 0x7FFFE0;       // 残高18bit分のみ論理積で取り出す
                    balance = balance >> 5;             // 5bit分ビットシフト
                // 残高表示
                    if(flag_detectnow!=1)printBalanceLCD("WAON", &balance,1,flag_detectnow);
                    flag_detectnow=1;
                }
            }
        } else if(felica.polling(COMPANY_ID_SYSTEM_CODE)){
            if(requestService(COMPANY_ID_SERVICE_CODE)){
            // Company Card
                if(readEncryption(COMPANY_ID_SERVICE_CODE, 0, buf)){
                    if(flag_detectnow!=1)printIDCard(buf,flag_detectnow);
                    for(int i=15;i<22;i++){
                        ret = ret*10+(buf[i]-0x30);
                    }
                    countnum=0;
                    flag_detectnow=1;
                }
            }
        } else {   // 不明カード表示(idm,pmm)
            int pos_x = 50, pos_y = 60;
            if(flag_detectnow==0)Buzzer_bubu(3);
            flag_detectnow=1;
            TFT.set_font( (unsigned char *)Prototype33x38 );
            TFT.locate(pos_x+10,pos_y+10);TFT.foreground(White);
            //TFT.printf("Card");
            TFT.printf("+/20"); // C(43) a(47) r(50) d(48)
            TFT.set_font( (unsigned char *)Prototype29x28 );
            TFT.foreground(GreenYellow);
            TFT.locate(pos_x+30,pos_y+55);TFT.printf("Unknown!");
    
            TFT.set_font( (unsigned char *)Consolas7x13 );
            TFT.foreground(White);
            TFT.locate(pos_x+10,pos_y + 90);
            TFT.printf("idm:");
            for (int i=0;i<=7;i++) TFT.printf("%02X",felica.idm[i]);
            TFT.locate(pos_x+10,pos_y + 102);
            TFT.printf("pmm:");
            for (int i=0;i<=7;i++) TFT.printf("%02X",felica.pmm[i]);
        }
      } else {      //デフォルト表示
        //TFT.cls();
        //flag_detectnow=0;
        //TFT.foreground( White );
        //TFT.locate(0,0);
        //TFT.set_font( (unsigned char *)Consolas9x16 );
        //TFT.printf("Card searching..");
        return ret;
      }
      while( felica.polling( 0xFFFF ) ){
          felica.rfOff();
          //wait_ms( POLLING_INTERVAL );
          thread_sleep_for(200);
      }
//    }
    return ret;
}