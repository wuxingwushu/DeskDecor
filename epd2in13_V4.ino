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
#include "OpenMeteo.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
// #include "WebDAV.h"

UBYTE *BlackImage;

NetworkCase CaseInfo;

WiFiUDP ntpUDP;
NTPClient TimeNTPClient(ntpUDP, "ntp.aliyun.com", 60*60*8, 60*1000);

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

  OpenMeteoInfo infoM;
  infoM.Success = false;
  CaseInfo = ConnectWIFI();    // 连接wifi
  if (CaseInfo == Network_Wed) // 开启Wed服务
  {
    String IPstr = WebServerFun();
    int SideSize = QR(IPstr.c_str(), PosRight);
    CN_Show(40, 0, "连接热点", EPD_2in13_V4_HEIGHT - SideSize);
    CN_Show(20, 24, "\"一言墨水屏\"", EPD_2in13_V4_HEIGHT - SideSize);
    CN_Show(0, 48, "再扫码进入Wed服务端进行设置。", EPD_2in13_V4_HEIGHT - SideSize);
    Debug("Network_Wed\n");
  }
  else if (CaseInfo == Network_Ok) // wifi连接成功
  {
    // 获取 一言内容 和 显示
    int InquireCount = 0;
    HitokotoInfo InfoD;
    InfoD.Success = false;
    while (!InfoD.Success)
    {
      ++InquireCount;
      InfoD = GetHitokoto();
      if (!InfoD.Success)
      {
        Debug("Error: GetHitokoto()\n");
        if (InquireCount > 10)
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

    // 天气信息
    InquireCount = 0;
    while (!infoM.Success)
    {
      ++InquireCount;
      infoM = GetOpenMeteo();
      if(!infoM.Success)
      {
        Debug("Error: GetOpenMeteo()\n");
        if (InquireCount > 10)
          break;
        DEV_Delay_ms(10);
      }
      else
      {
        
        String WeatherInfo = GetMeteoToString(infoM.Weather) + "," + infoM.Temperature;
        Debug("\n" + String(WeatherInfo) + "\n");
        CN_Show(0, 76, WeatherInfo.c_str());
      }
    }

    // 获取时间
    TimeNTPClient.begin();
    TimeNTPClient.update();
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
  Debug("\t" + String(Power) + "\n");
  for (int i = 0; i < Power; ++i)
    Paint_SetPixel(i, 121, BLACK);

  // 判斷休眠時間
  unsigned int DelayTime;
  if (CaseInfo != Network_Wed) // 非 Wed服務模式
  {
    EEPROM.get(SleepValueAddr, DelayTime);// 獲取一言刷新間隔時間（分）
    if(CaseInfo == Network_Ok){// 判斷是否有網絡
      // 獲取設置的工作時間範圍
      unsigned char StartHours, StartMinutes, EndHours, EndMinutes;
      EEPROM.get(StartTimeHoursAddr, StartHours);// 獲取開始 時
      EEPROM.get(StartTimeMinutesAddr, StartMinutes);// 獲取開始 分
      EEPROM.get(EndTimeHoursAddr, EndHours);// 獲取結束 時
      EEPROM.get(EndTimeMinutesAddr, EndMinutes);// 獲取結束 分
      // 獲取當前時間
      int TimeH = TimeNTPClient.getHours();// 获取时
      int TimeM = TimeNTPClient.getMinutes();// 获取分
      String DormantTime = "休眠到 ";
      if(StartHours < 10){DormantTime += "0" + String(StartHours);}else{DormantTime += String(StartHours);}
      DormantTime += ":";
      if(StartMinutes < 10){DormantTime += "0" + String(StartMinutes);}else{DormantTime += String(StartMinutes);}
      // 判斷是否在工作時間範圍外
      if(TimeH >= EndHours){
        if(!((TimeH == EndHours) && (TimeM < EndMinutes))){
          DelayTime = (24 - TimeH + StartHours) * 60 - TimeM;// 工作範圍外到工作時間開始的時間差（分）
          CN_Show(125, 76, DormantTime.c_str());// 開始時間顯示在屏幕上
        }
      }else if(TimeH <= StartHours){
        if((TimeH == StartHours) && (TimeM < StartMinutes)){
          DelayTime = (StartHours - TimeH) * 60 - TimeM;// 工作範圍外到工作時間開始的時間差（分）
          CN_Show(125, 76, DormantTime.c_str());// 開始時間顯示在屏幕上
        }
      }

      // 断开连接
      WiFi.disconnect(true);
    }
    Debug("休眠时间：" + String(DelayTime) + "\n");
  }

  // 刷新屏幕显示内容
  for (int i = 0; i < 5; ++i)
  {
    RenovateScreen(BlackImage);
    DEV_Delay_ms(10);
  }

  if (CaseInfo != Network_Wed)
  {
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
