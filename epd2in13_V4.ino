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
#include "PresentTime.h"
// #include "WebDAV.h"

// 显示缓冲
UBYTE *BlackImage;

// 当前网络状态
NetworkCase CaseInfo;

void setup() {
  // 初始化 模块或设备
  DEV_Module_Init();
  // 初始化 EEPROM
  EEPROM.begin(EepromBufferSize);
  // 初始化 屏幕
  EPD_2in13_V4_Init();
  // 初始化屏幕内容
  const UWORD Imagesize = ((EPD_2in13_V4_WIDTH % 8 == 0) ? (EPD_2in13_V4_WIDTH / 8) : (EPD_2in13_V4_WIDTH / 8 + 1)) * EPD_2in13_V4_HEIGHT;
  if ((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
    Debug("Failed to apply for black memory...\n");
    while (1)
      ;
  }
  Paint_NewImage(BlackImage, EPD_2in13_V4_WIDTH, EPD_2in13_V4_HEIGHT, 90, WHITE);
  Paint_Clear(WHITE);
  // 初始化 ADC电压检测
  ADC_Init();
  // 初始化 文件管理系统 （需要字体文件）
  if (!SPIFFS.begin(true)) {
    Debug("An Error has occurred while mounting SPIFFS\n");
    return;
  }
  DEV_Delay_ms(10);

  unsigned int DelayTime;                 // 延迟时间（分）
  EEPROM.get(SleepValueAddr, DelayTime);  // 獲取一言刷新間隔時間（分）
  CaseInfo = ConnectWIFI();               // 连接wifi
  if (CaseInfo == Network_Wed)            // 开启Wed服务
  {
    String IPstr = WebServerFun();               // 开启服务
    int SideSize = QR(IPstr.c_str(), PosRight);  // 显示服务 IP
    // 显示在屏幕
    CN_Show(40, 0, "连接热点", EPD_2in13_V4_HEIGHT - SideSize);
    CN_Show(20, 24, "\"一言墨水屏\"", EPD_2in13_V4_HEIGHT - SideSize);
    CN_Show(0, 48, "再扫码进入Wed服务端进行设置。", EPD_2in13_V4_HEIGHT - SideSize);
    Debug("Network_Wed\n");
  } else if (CaseInfo == Network_Ok)  // wifi连接成功
  {
    // 获取 一言内容 和 显示
    HitokotoInfo InfoD = GetHitokoto(10);  // 获取 一言 内容
    if (InfoD.Success) {
      Debug("Hitokoto OK!\n");
      CN_Show(40, 0, InfoD.hitokoto);
      CN_Show(0, 100, InfoD.from);
    } else {
      Debug("Error: GetHitokoto()\n");
    }

    // 天气信息
    OpenMeteoInfo infoM = GetOpenMeteo();
    if (infoM.Success) {
      String WeatherInfo = GetMeteoToString(infoM.Weather) + "," + infoM.Temperature;
      Debug(String(WeatherInfo) + "\n");
      CN_Show(0, 76, WeatherInfo.c_str());
    } else {
      Debug("Error: GetOpenMeteo()\n");
    }

    // 获取时间
    RequestPresentTime();
    PresentTimeInfo InfoPT = GetDelayTime();  // 时间信息
    if (InfoPT.Success) {
      DelayTime = InfoPT.PresentTime;
      CN_Show(125, 76, InfoPT.PresentStr.c_str());
    }

    // 断开连接
    WiFi.disconnect(true);
  } else  // wifi连接失败
  {
    // 从 EEPROM 中读取数据
    int Index;
    EEPROM.get(ImgIndexAddr, Index);  // 读取flase;
    ++Index;
    if (Index >= 8)
      Index = 0;
    EEPROM.put(ImgIndexAddr, Index);  // 存储
    EEPROM.commit();
    // 显示图片
    Img_Show(EPD_2in13_V4_WIDTH, EPD_2in13_V4_HEIGHT, ImgD[Index]);
  }

  // 获取电压，显示电压
  int Power = ReadADC() * EPD_2in13_V4_HEIGHT;
  if (Power > EPD_2in13_V4_HEIGHT)
    Power = EPD_2in13_V4_HEIGHT;
  Debug("电量长度：" + String(Power) + "\n");
  for (int i = 0; i < Power; ++i)
    Paint_SetPixel(i, 121, BLACK);

  // 刷新屏幕显示内容
  for (int i = 0; i < 5; ++i) {
    RenovateScreen(BlackImage);
    DEV_Delay_ms(10);
  }

  if (CaseInfo != Network_Wed) {
    Debug("休眠时间：" + String(DelayTime) + "分\n");
    DelayTime = DelayTime * 60 * 1000000;
    Debug(String(DelayTime) + "us\n");
    // 设置唤醒时间
    esp_sleep_enable_timer_wakeup(DelayTime);
    // 进入深度睡眠状态
    esp_deep_sleep_start();
  }
}

void loop() {
  if (CaseInfo == Network_Wed) {
    // 处理来自客户端的HTTP请求
    server.handleClient();
  }
}
