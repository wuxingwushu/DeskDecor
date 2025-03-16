/* Includes ------------------------------------------------------------------*/
#include "DEV_Config.h"
#include "EPD_2in13_V4.h"
#include "GUI_Paint.h"
#include <stdlib.h>
#include <Arduino.h>
#include "Sentence.h"
#include "ADC.h"
#include <SPIFFS.h>
#include "ImageData.h"
#include "Network.h"
#include <EEPROM.h>
#include "EepromString.h"
#include "OpenMeteo.h"
#include "PresentTime.h"
#include "FileSystem.h"
// #include "WebDAV.h"

// 当前网络状态
NetworkCase CaseInfo;

void setup()
{
  // 开始计算耗时
  unsigned int WorkConsumeTime = millis();
  unsigned int ConsumeTime = millis();
  // 初始化 模块或设备
  DEV_Module_Init();
  Debug("开机\n");
  // 初始化 EEPROM
  EEPROM.begin(EepromBufferSize);
  // 初始化 屏幕
  EPD_2in13_V4_Init();
  // 初始化屏幕内容
  UBYTE BlackImage[((EPD_2in13_V4_WIDTH % 8 == 0) ? (EPD_2in13_V4_WIDTH / 8) : (EPD_2in13_V4_WIDTH / 8 + 1)) * EPD_2in13_V4_HEIGHT]; // 显示缓冲
  Paint_NewImage(BlackImage, EPD_2in13_V4_WIDTH, EPD_2in13_V4_HEIGHT, 90, WHITE);
  Paint_Clear(WHITE);
  // 初始化 ADC电压检测
  ADC_Init();
  // 初始化 文件管理系统 （需要字体文件）
  if (!SPIFFS.begin(true))
  {
    Debug("An Error has occurred while mounting SPIFFS\n");
    return;
  }
#if 0 // 测试代码，可以删除
  writeStringToEEPROM(WifiNameAddr + (WiFiStrInterval * 0), "道生");
  writeStringToEEPROM(WifiPassAddr + (WiFiStrInterval * 0), "369784512");
  writeStringToEEPROM(WifiNameAddr + (WiFiStrInterval * 1), "KT-2.4G");
  writeStringToEEPROM(WifiPassAddr + (WiFiStrInterval * 1), "aurex123456");
  writeStringToEEPROM(WifiNameAddr + (WiFiStrInterval * 2), "CMCC-U55E");
  writeStringToEEPROM(WifiPassAddr + (WiFiStrInterval * 2), "eku7dx5f");
  writeStringToEEPROM(WifiNameAddr + (WiFiStrInterval * 3), "888");
  writeStringToEEPROM(WifiPassAddr + (WiFiStrInterval * 3), "Huangze123");
  writeStringToEEPROM(WifiNameAddr + (WiFiStrInterval * 4), "USER_128978");
  writeStringToEEPROM(WifiPassAddr + (WiFiStrInterval * 4), "70438480");
  ShowFileInfo();
#endif
  DEV_Delay_ms(10);

  bool TimeUpDateBool = false;
  unsigned short DelayTime;              // 延迟时间（分）
  EEPROM.get(SleepValueAddr, DelayTime); // 獲取一言刷新間隔時間（分）
  CaseInfo = ConnectWIFI();              // 连接wifi
  if (CaseInfo == Network_Wed)           // 开启Wed服务
  {
    String IPstr = WebServerFun();              // 开启服务
    int SideSize = QR(IPstr.c_str(), PosRight); // 显示服务 IP
    // 显示在屏幕
    CN_Show(40, 0, "连接热点", EPD_2in13_V4_HEIGHT - SideSize);
    CN_Show(20, 24, "\"一言墨水屏\"", EPD_2in13_V4_HEIGHT - SideSize);
    CN_Show(0, 48, "再扫码进入Wed服务端进行设置。", EPD_2in13_V4_HEIGHT - SideSize);
    Debug("Network_Wed\n");
  }
  else if (CaseInfo == Network_Ok) // wifi连接成功
  {
    // 获取 一言内容 和 显示
    SentenceInfo InfoD = GetSentence(); // 获取 一言 内容
    if (InfoD.Success)
    {
      if (InfoD.StrSize >= 120)
      {
        Debug("句子过长，再获取一次\n");
        InfoD = GetSentence();
      }
      Debug("Hitokoto OK!\n");
      CN_Show(34, 0, InfoD.hitokoto);
      CN_Show(0, 100, InfoD.from);
    }
    else
    {
      Debug("Error: GetHitokoto()\n");
      CN_Show(34, 0, "获取句子失败! 是没网还是BUG……");
      CN_Show(0, 100, "开发者");
    }

    // 天气信息
    OpenMeteoInfo infoM = GetOpenMeteo();
    if (infoM.Success)
    {
      String WeatherInfo = GetMeteoToString(infoM.Weather) + "," + infoM.Temperature;
      Debug(String(WeatherInfo) + "\n");
      CN_Show(0, 76, WeatherInfo.c_str());
    }
    else
    {
      CN_Show(0, 76, "天气罢工啦!");
    }

    // 获取时间
    TimeUpDateBool = RequestPresentTime();
    if (TimeUpDateBool)
    {
      ConsumeTime = millis();
      PresentTimeInfo InfoPT = GetDelayTime(); // 时间信息
      if (InfoPT.Success)
      {
        DelayTime = InfoPT.PresentTime;
        CN_Show(125, 76, InfoPT.PresentStr.c_str());
      }
    }

    // 断开连接
    WiFi.disconnect(true);
  }
  else if (CaseInfo == Network_Not) // wifi连接失败
  {
    // 从 EEPROM 中读取数据
    int Index;
    EEPROM.get(ImgIndexAddr, Index); // 读取flase;
    ++Index;
    if (Index >= 8)
      Index = 0;
    EEPROM.put(ImgIndexAddr, Index); // 存储
    // 显示图片
    Img_Show(EPD_2in13_V4_WIDTH, EPD_2in13_V4_HEIGHT, ImgD[Index]);
  }
  else
  {
    DelayTime = 60;
    CN_Show(40, 0, "没有WiFi，我咋子工作嘛！");
    CN_Show(0, 100, "开发者");
  }

  // 获取电压，显示电压
  int Power = ReadADC() * EPD_2in13_V4_HEIGHT;
  if (Power > EPD_2in13_V4_HEIGHT)
    Power = EPD_2in13_V4_HEIGHT;
  Debug("电量长度：" + String(Power) + "\n");
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
    if (!TimeUpDateBool)
    {
      EEPROM.get(PresentTimeHoursAddr, TimeH);
      EEPROM.get(PresentTimeMinutesAddr, TimeM);
      EEPROM.get(PresentTimeSecondAddr, TimeS);
      PresentTimeInfo InfoPT = GetDelayTime(TimeH, TimeM);
      if (InfoPT.Success)
      {
        DelayTime = InfoPT.PresentTime;
        CN_Show(125, 76, InfoPT.PresentStr.c_str());
      }
    }

    // 更新离线时间(计算下次唤醒的时间)
    ConsumeTime = millis() - ConsumeTime;
    Debug("耗时:" + String(ConsumeTime) + "ms\n");
    TimeS += ConsumeTime / 1000 + 1;
    if (TimeS >= 60)
    {
      TimeS -= 60;
      ++DelayTime;
    }
    unsigned int TimeML = TimeM;
    TimeML += DelayTime;
    while (TimeML >= 60)
    {
      TimeML -= 60;
      ++TimeH;
      if (TimeH >= 24)
      {
        TimeH -= 24;
      }
    }
    TimeM = TimeML;
    EEPROM.put(PresentTimeHoursAddr, TimeH);
    EEPROM.put(PresentTimeMinutesAddr, TimeM);
    EEPROM.put(PresentTimeSecondAddr, TimeS);
    EEPROM.commit();
    DEV_Delay_ms(10);
    Debug("唤醒时间：" + String(TimeH) + ":" + String(TimeM) + ":" + String(TimeS) + "\n");

    // 休眠
    Debug("休眠时间：" + String(DelayTime) + "分\n");
    unsigned long long int SleepTime = ((unsigned long long int)DelayTime) * 60 * 1000000;
    Debug(String(SleepTime) + "us\n");
    Debug("总耗时:" + String(millis() - WorkConsumeTime) + "ms\n");
    // 设置唤醒时间
    esp_sleep_enable_timer_wakeup(SleepTime);
    // 进入深度睡眠状态
    esp_deep_sleep_start();
  }
}

void loop() {}
