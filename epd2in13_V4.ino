/* Includes ------------------------------------------------------------------*/
#include "DEV_Config.h"
#include "EPD_2in13_V4.h"
#include "GUI_Paint.h"
#include <stdlib.h>
#include <Arduino.h>
#include "Hitokoto.h"
#include "ADC.h"
#include <SPIFFS.h>
#include "ImageData.h"
#include "Network.h"
#include <EEPROM.h>
#include "EepromString.h"
// #include "WebDAV.h"

UBYTE *BlackImage;

NetworkCase CaseInfo;

void setup()
{
  // 初始化 模块或设备
  DEV_Module_Init();
  // 初始化 EEPROM
  EEPROM.begin(EepromBufferSize);
  // 初始化 屏幕
  EPD_2in13_V4_Init();
  // 初始化屏幕内容
  const UWORD Imagesize = ((EPD_2in13_V4_WIDTH % 8 == 0) ? (EPD_2in13_V4_WIDTH / 8) : (EPD_2in13_V4_WIDTH / 8 + 1)) * EPD_2in13_V4_HEIGHT;
  if ((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL)
  {
    Debug("Failed to apply for black memory...\r\n");
    while (1)
      ;
  }
  Paint_NewImage(BlackImage, EPD_2in13_V4_WIDTH, EPD_2in13_V4_HEIGHT, 90, WHITE);
  Paint_Clear(WHITE);
  // 初始化 ADC电压检测
  ADC_Init();
  // 初始化 文件管理系统 （需要字体文件）
  if (!SPIFFS.begin(true))
  {
    Debug("An Error has occurred while mounting SPIFFS");
    return;
  }
  DEV_Delay_ms(10);

  CaseInfo = ConnectWIFI();    // 连接wifi
  if (CaseInfo == Network_Wed) // 开启Wed服务
  {
    String IPstr = WebServerFun();
    int Bian = QR(IPstr.c_str(), PosRight);
    CN_Show(40, 0, "连接热点", EPD_2in13_V4_HEIGHT - Bian);
    CN_Show(20, 24, "\"一言墨水屏\"", EPD_2in13_V4_HEIGHT - Bian);
    CN_Show(0, 48, "再扫码进入Wed服务端进行设置。", EPD_2in13_V4_HEIGHT - Bian);
    Debug("Network_Wed\n");
  }
  else if (CaseInfo == Network_Ok) // wifi连接成功
  {
    // 获取 一言内容 和 显示
    int HitokotoGet = 0;
    HitokotoInfo InfoD;
    InfoD.Success = false;
    while (!InfoD.Success)
    {
      ++HitokotoGet;
      InfoD = GetHitokoto();
      if (!InfoD.Success)
      {
        Debug("Error: GetHitokoto()\n");
        if (HitokotoGet > 10)
          break;
        DEV_Delay_ms(10);
      }
      else
      {
        Debug("Hitokoto OK!\n");
        CN_Show(40, 0, InfoD.hitokoto);
        CN_Show(0, 100, InfoD.from);
      }
    }

    // 断开连接
    WiFi.disconnect(true);
  }
  else // wifi连接失败
  {
    // 从 EEPROM 中读取数据
    int Index;
    EEPROM.get(ImgIndexAddr, Index); // 读取flase;
    ++Index;
    if (Index >= 8)
      Index = 0;
    EEPROM.put(ImgIndexAddr, Index); // 存储
    EEPROM.commit();
    // 显示图片
    Img_Show(EPD_2in13_V4_WIDTH, EPD_2in13_V4_HEIGHT, ImgD[Index]);
  }

  // 获取电压，显示电压
  int Power = ReadADC() * EPD_2in13_V4_HEIGHT;
  if (Power > EPD_2in13_V4_HEIGHT)
    Power = EPD_2in13_V4_HEIGHT;
  Debug("\t");
  Debug(Power);
  Debug("\n");
  for (int i = 0; i < Power; ++i)
    Paint_SetPixel(i, 121, BLACK);

  // 刷新屏幕显示内容
  for (int i = 0; i < 5; ++i)
  {
    RenovateScreen(BlackImage);
    DEV_Delay_ms(10);
  }

  if (CaseInfo != Network_Wed)
  {
    int DelayTime;
    EEPROM.get(SleepValueAddr, DelayTime);
    Debug("休眠时间：");
    Debug(DelayTime);
    // 设置唤醒时间
    esp_sleep_enable_timer_wakeup(DelayTime * 60 * 1000000);
    // 进入深度睡眠状态
    esp_deep_sleep_start();
  }
}

void loop()
{
  if (CaseInfo == Network_Wed)
  {
    // 处理来自客户端的HTTP请求
    server.handleClient();
  }
}
