/* Includes ------------------------------------------------------------------*/
#include "DEV_Config.h"
#include "EPD_2in13_V4.h"
#include "GUI_Paint.h"
#include <stdlib.h>
#include <Arduino.h>
#include "Hitokoto.h"
#include "ADC.h"
#include <SPIFFS.h>
#include "qrcodegen.h"

//const char* ssid = "道生";
//const char* password = "369784512";
const char* ssid = "USER_028892";
const char* password = "09577678";



UBYTE *BlackImage;
uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];

void Paint_SetBlock(UWORD Xpoint, UWORD Ypoint, UWORD BlockSize, UWORD Color, UWORD Xp){
  Xpoint *= BlockSize;
  Ypoint *= BlockSize;
  for(int x = Xpoint; x <= (Xpoint + BlockSize); ++x){
    for(int y = Ypoint; y <= (Ypoint + BlockSize); ++y){
      Paint_SetPixel(x + Xp, y, Color);
    }
  }
}

void QR(){
  const char *text = "https://github.com/wuxingwushu/";
	enum qrcodegen_Ecc errCorLvl = qrcodegen_Ecc_LOW;
	
	bool ok = qrcodegen_encodeText(text, tempBuffer, qrcode, errCorLvl,
		qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
	if (!ok)
    return;
    
  int size = qrcodegen_getSize(qrcode);
  int BlockSize = EPD_2in13_V4_WIDTH / size;
  int bianchuang = 0;
  int XPos = (EPD_2in13_V4_HEIGHT - (size * BlockSize)) / 2;
  for(int x = -bianchuang; x < size + bianchuang; ++x){
    for(int y = -bianchuang; y < size + bianchuang; ++y){
      Paint_SetBlock(x + bianchuang, y + bianchuang, BlockSize, qrcodegen_getModule(qrcode, x, y) ? BLACK : WHITE, XPos);
    }
  }
}

/* Entry point ----------------------------------------------------------------*/
void setup()
{
  //EEPROM.begin(4096); 
	DEV_Module_Init();
  DEV_Delay_ms(100);

  if(!SPIFFS.begin(true)){
    Debug("An Error has occurred while mounting SPIFFS");
    return;
  }
  DEV_Delay_ms(100);

	EPD_2in13_V4_Init();
  DEV_Delay_ms(100);

	//Create a new image cache
	
	UWORD Imagesize = ((EPD_2in13_V4_WIDTH % 8 == 0)? (EPD_2in13_V4_WIDTH / 8 ): (EPD_2in13_V4_WIDTH / 8 + 1)) * EPD_2in13_V4_HEIGHT;
	if((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) 
	{
		printf("Failed to apply for black memory...\r\n");
		while (1);
	}
	
	Paint_NewImage(BlackImage, EPD_2in13_V4_WIDTH, EPD_2in13_V4_HEIGHT, 90, WHITE);
  DEV_Delay_ms(100);
  //free(BlackImage);
  Debug("初始化\n");

  Paint_Clear(WHITE);
  QR();
  for(int i = 0; i < 6; ++i){
    EPD_2in13_V4_Display_Partial(BlackImage);
    DEV_Delay_ms(100);
  }

  ADC_Init();
  DEV_Delay_ms(100);

  pinMode(12, INPUT_PULLUP);
  DEV_Delay_ms(100);
}

int Pin_ = 0;
bool WifiDuan = true;//是否需要清屏
int DianYa = 0;
bool WIFIConnectBool = false;

bool ConnectWIFI(){
  // 启用WiFi模块
  WiFi.mode(WIFI_STA);
  // 连接WiFi
  WiFi.begin(ssid, password);

  int cishu = 0;
  Debug("\n");
  while (WiFi.status() != WL_CONNECTED) {
    ++cishu;
    if(cishu > 10){
      if(WifiDuan){
        WifiDuan = false;
        Paint_Clear(WHITE);
        for(int i = 0; i < 10; ++i){
          EPD_2in13_V4_Display_Partial(BlackImage);
          DEV_Delay_ms(100);
        }
      }
      

      // 断开WiFi连接
      WiFi.disconnect(true);
      // 关闭WiFi模块
      WiFi.mode(WIFI_OFF);

      return false;
    }
    delay(1000);
    Debug(".");
  }

  return true;
}

bool QRBool = false;
/* The main loop -------------------------------------------------------------*/
void loop()
{
  int PinL;

  if(!QRBool){
    WIFIConnectBool = ConnectWIFI();
  }else{
    WIFIConnectBool = false;
  }
  

  if(WIFIConnectBool){
    HitokotoInfo InfoD;
    InfoD.Success = false;
    while(!InfoD.Success){
      InfoD = GetHitokoto();
      WifiDuan = true;
      if(!InfoD.Success){
       DEV_Delay_ms(100);
        Debug("\n");
        Debug("Error: GetHitokoto()");
      }
    }

    // 断开WiFi连接
    WiFi.disconnect(true);
    // 关闭WiFi模块
    WiFi.mode(WIFI_OFF);
  

    Paint_Clear(WHITE);
  
    CN_Show(40, 0, InfoD.hitokoto);
    CN_Show(0, 100, InfoD.from);

    int Dian = ReadADC() * EPD_2in13_V4_HEIGHT;
    if(Dian > EPD_2in13_V4_HEIGHT){
      Dian = EPD_2in13_V4_HEIGHT;
    }
    Debug("\t");
    Debug(Dian);
    Debug("\n");
    for(int i = 0; i < Dian; ++i){
      Paint_SetPixel(i, 121, BLACK);
    }

    for(int i = 0; i < 6; ++i){
      EPD_2in13_V4_Display_Partial(BlackImage);
      DEV_Delay_ms(100);
    }
  }else if(QRBool){
    Paint_Clear(WHITE);
    QR();
    for(int i = 0; i < 6; ++i){
      EPD_2in13_V4_Display_Partial(BlackImage);
      DEV_Delay_ms(100);
    }
    PinL = 1;
    while(PinL){
      PinL = digitalRead(12);
      DEV_Delay_ms(200);
    }
    QRBool = false;
    return;
  }

  
  
  
  for(int i = 0; i < 60 * 2 * 10; ++i){
    PinL = (digitalRead(12) == 1) ? 0 : 1;
    if(PinL && (Pin_ != PinL)){
      Debug("\n开始\n");
      int time = 0;
      while(PinL){
        PinL = (digitalRead(12) == 1) ? 0 : 1;
        ++time;
        if(time > 10){
          QRBool = true;
          break;
        }
        DEV_Delay_ms(100);
      }
      break;
    }
    Pin_ = PinL;
    DEV_Delay_ms(100);
  }
}
