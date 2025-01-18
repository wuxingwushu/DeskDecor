/* Includes ------------------------------------------------------------------*/
#include "DEV_Config.h"
#include "EPD_2in13_V4.h"
#include "GUI_Paint.h"
#include <stdlib.h>
#include <Arduino.h>
#include "Hitokoto.h"
#include "ADC.h"
#include <SPIFFS.h>
#include <EEPROM.h>
#include "ImageData.h"
// #include "WebDAV.h"

//const char *ssid = "KT-2.4G", *password = "aurex123456";
// const char* ssid = "CMCC-U55E", *password = "eku7dx5f";
// const char* ssid = "道生", *password = "369784512";
 const char* ssid = "USER_128978", *password = "70438480";

#define DelayMinute 5

UBYTE *BlackImage;

bool ConnectWIFI()
{
  // 启用WiFi模块
  WiFi.mode(WIFI_STA);
  // 连接WiFi
  WiFi.begin(ssid, password);

  int cishu = 0;
  Debug("\n");
  while (WiFi.status() != WL_CONNECTED)
  {
    ++cishu;
    if (cishu > 100)
    { // 几次后没法连接判定为没有网络
      return false;
    }
    DEV_Delay_ms(100);
    Debug(".");
  }

  return true;
}

/* Entry point ----------------------------------------------------------------*/
void setup()
{
  // 初始化 模块或设备
  DEV_Module_Init();
  DEV_Delay_ms(10);
  // 初始化 文件管理系统 （需要字体文件）
  if (!SPIFFS.begin(true))
  {
    Debug("An Error has occurred while mounting SPIFFS");
    return;
  }
  DEV_Delay_ms(10);
  // 初始化 屏幕
  EPD_2in13_V4_Init();
  DEV_Delay_ms(10);
  // 初始化 ADC电压检测
  ADC_Init();
  DEV_Delay_ms(10);
  // 初始化 12引脚口
  // pinMode(12, INPUT_PULLUP);
  // DEV_Delay_ms(10);

  // 初始化屏幕内容
  UWORD Imagesize = ((EPD_2in13_V4_WIDTH % 8 == 0) ? (EPD_2in13_V4_WIDTH / 8) : (EPD_2in13_V4_WIDTH / 8 + 1)) * EPD_2in13_V4_HEIGHT;
  if ((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL)
  {
    printf("Failed to apply for black memory...\r\n");
    while (1)
      ;
  }
  Paint_NewImage(BlackImage, EPD_2in13_V4_WIDTH, EPD_2in13_V4_HEIGHT, 90, WHITE);
  DEV_Delay_ms(10);
  Paint_Clear(WHITE);

  // 获取电压，显示电压
  int Dian = ReadADC() * EPD_2in13_V4_HEIGHT;
  if (Dian > EPD_2in13_V4_HEIGHT)
  {
    Dian = EPD_2in13_V4_HEIGHT;
  }
  Debug("\t");
  Debug(Dian);
  Debug("\n");

  // 是否可以连接WIFI
  if (!ConnectWIFI())
  {
    // 初始化 EEPROM
    EEPROM.begin(4);
    // 从 EEPROM 中读取数据
    int value;
    EEPROM.get(0, value); // 读取flase;
    ++value;
    if (value >= 8)
    {
      value = 0;
    }
    // 显示图片
    Img_Show(EPD_2in13_V4_WIDTH, EPD_2in13_V4_HEIGHT, ImgD[value]);
    EEPROM.put(0, value); // 存储
    EEPROM.commit();
  }else{
    // 获取 一言内容 和 显示
    int HitokotoCiShu = 0;
    HitokotoInfo InfoD;
    InfoD.Success = false;
    while (!InfoD.Success)
    {
      ++HitokotoCiShu;
      InfoD = GetHitokoto();
      if (!InfoD.Success)
      {
        DEV_Delay_ms(100);
        Debug("Error: GetHitokoto()\n");
        if (HitokotoCiShu > 10)
        {
          break;
        }
      }
      else
      {
        Debug("Hitokoto OK!\n");
        CN_Show(40, 0, InfoD.hitokoto);
        CN_Show(0, 100, InfoD.from);
      }
    }
  }

  

  // 断开WiFi连接
  WiFi.disconnect(true);
  // 关闭WiFi模块
  WiFi.mode(WIFI_OFF);

  for (int i = 0; i < Dian; ++i)
  {
    Paint_SetPixel(i, 121, BLACK);
  }

  // 刷新屏幕显示内容
  for (int i = 0; i < 5; ++i)
  {
    RenovateScreen(BlackImage);
    DEV_Delay_ms(100);
  }
  // 设置GPIO唤醒
  // esp_sleep_enable_ext0_wakeup(GPIO_NUM_12, LOW);//12脚低电平唤醒
  // 设置唤醒时间为两分钟
  esp_sleep_enable_timer_wakeup(DelayMinute * 60 * 1000000);
  // 进入深度睡眠状态
  esp_deep_sleep_start();
}

void loop()
{
}
