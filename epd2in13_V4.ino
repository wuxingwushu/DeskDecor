/* Includes ------------------------------------------------------------------*/
#include "DEV_Config.h"
#include "EPD_2in13_V4.h"
#include "GUI_Paint.h"
#include <stdlib.h>
#include <Arduino.h>
#include "Hitokoto.h"
#include "ADC.h"
#include <SPIFFS.h>


//const char* ssid = "道生";
//const char* password = "369784512";
const char* ssid = "USER_028892";
const char* password = "09577678";

UBYTE *BlackImage;

bool ConnectWIFI(){
  // 启用WiFi模块
  WiFi.mode(WIFI_STA);
  // 连接WiFi
  WiFi.begin(ssid, password);

  int cishu = 0;
  Debug("\n");
  while (WiFi.status() != WL_CONNECTED) {
    ++cishu;
    if(cishu > 10){//十次后没法连接判定为没有网络
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




/* Entry point ----------------------------------------------------------------*/
void setup()
{
  //初始化 模块或设备
	DEV_Module_Init();
  DEV_Delay_ms(100);
  //初始化 文件管理系统 （需要字体文件）
  if(!SPIFFS.begin(true)){
    Debug("An Error has occurred while mounting SPIFFS");
    return;
  }
  DEV_Delay_ms(100);
  //初始化 屏幕
	EPD_2in13_V4_Init();
  DEV_Delay_ms(100);
  //初始化 ADC电压检测
  ADC_Init();
  DEV_Delay_ms(100);
  //初始化 12引脚口
  pinMode(12, INPUT_PULLUP);
  DEV_Delay_ms(100);

  //初始化屏幕内容
  UWORD Imagesize = ((EPD_2in13_V4_WIDTH % 8 == 0)? (EPD_2in13_V4_WIDTH / 8 ): (EPD_2in13_V4_WIDTH / 8 + 1)) * EPD_2in13_V4_HEIGHT;
	if((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL)
	{
		printf("Failed to apply for black memory...\r\n");
		while (1);
	}
	Paint_NewImage(BlackImage, EPD_2in13_V4_WIDTH, EPD_2in13_V4_HEIGHT, 90, WHITE);
  DEV_Delay_ms(10);
  Paint_Clear(WHITE);


  //获取电压，显示电压
  int Dian = ReadADC();
  Num_Show(170, 100, Dian);
  Dian = ((float)Dian / VoltageRange) * EPD_2in13_V4_HEIGHT;
  if(Dian > EPD_2in13_V4_HEIGHT){
    Dian = EPD_2in13_V4_HEIGHT;
  }
  Debug("\t");
  Debug(Dian);
  Debug("\n");
  for(int i = 0; i < Dian; ++i){
    Paint_SetPixel(i, 121, BLACK);
  }
  

  //是否可以连接WIFI
  if(!ConnectWIFI()){
    //刷新屏幕 只显示电量
    for(int i = 0; i < 6; ++i){
      EPD_2in13_V4_Display_Partial(BlackImage);
      DEV_Delay_ms(100);
    }
    // 设置GPIO唤醒
    //esp_sleep_enable_ext0_wakeup(GPIO_NUM_12, LOW);//12脚低电平唤醒
    // 设置唤醒时间为两分钟
    esp_sleep_enable_timer_wakeup(2 * 60 * 1000000);
    // 进入深度睡眠状态
    esp_deep_sleep_start();
  }

  //获取 一言内容 和 显示
  int HitokotoCiShu = 0;
  HitokotoInfo InfoD;
  InfoD.Success = false;
  while(!InfoD.Success){
    ++HitokotoCiShu;
    InfoD = GetHitokoto();
    if(!InfoD.Success){
      DEV_Delay_ms(100);
      Debug("Error: GetHitokoto()\n");
      if(HitokotoCiShu > 10){
        break;
      }
    }else{
      Debug("Hitokoto OK!\n");
      CN_Show(40, 0, InfoD.hitokoto);
      //CN_Show(0, 100, InfoD.from);
    }
  }
  // 断开WiFi连接
  WiFi.disconnect(true);
  // 关闭WiFi模块
  WiFi.mode(WIFI_OFF);

  
  //刷新屏幕显示内容
  for(int i = 0; i < 6; ++i){
    EPD_2in13_V4_Display_Partial(BlackImage);
    DEV_Delay_ms(100);
  }
  // 设置GPIO唤醒
  //esp_sleep_enable_ext0_wakeup(GPIO_NUM_12, LOW);//12脚低电平唤醒
  // 设置唤醒时间为两分钟
  esp_sleep_enable_timer_wakeup(2 * 60 * 1000000);
  // 进入深度睡眠状态
  esp_deep_sleep_start();
}

void loop()
{
  
}
