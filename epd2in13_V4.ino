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
UBYTE *BlackImage; // 显示缓冲

#define Bluetooth 0

#if Bluetooth

#include "BluetoothSerial.h"

BluetoothSerial SerialBT;
const char* deviceName = "ESP32-BT";  // 自定义蓝牙设备名称

#endif

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
  const UWORD Imagesize = ((EPD_2in13_V4_WIDTH % 8 == 0) ? (EPD_2in13_V4_WIDTH / 8) : (EPD_2in13_V4_WIDTH / 8 + 1)) * EPD_2in13_V4_HEIGHT;
  if ((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL)
  {
    Debug("Failed to apply for black memory...\n");
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
  PresentTimeInfo InfoPT;
  unsigned short DelayTime;              // 延迟时间（分）
  EEPROM.get(SleepValueAddr, DelayTime); // 獲取一言刷新間隔時間（分）
  CaseInfo = ConnectWIFI();              // 连接wifi
  if (CaseInfo == Network_Wed)           // 开启Wed服务
  {
#if Bluetooth

    SerialBT.begin(deviceName);      // 启动蓝牙
    Debug("蓝牙已启动，等待连接...\n");
    Debug("设备名称：");
    Debug(deviceName);
    Debug("\n");

#endif
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

    

    // 获取时间
    TimeUpDateBool = RequestPresentTime();
    if (TimeUpDateBool)
    {
      ConsumeTime = millis();
      InfoPT = GetDelayTime(); // 时间信息
      if (InfoPT.Success)
      {
        DelayTime = InfoPT.PresentTime;
      }
    }
    if (!InfoPT.Success) {
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
  ShowDecimalSystem(0, 6, Power);
  if (Power > EPD_2in13_V4_HEIGHT)
    Power = EPD_2in13_V4_HEIGHT;
  Debug("电量长度：" + String(Power) + "\n");
  for (int i = 0; i < Power; ++i)
    Paint_SetPixel(i, 121, BLACK);

  if (CaseInfo != Network_Wed)
  {
    if (!TimeUpDateBool)
    {
      EEPROM.get(DayAddr, TimeD);
      EEPROM.get(PresentTimeHoursAddr, TimeH);
      EEPROM.get(PresentTimeMinutesAddr, TimeM);
      EEPROM.get(PresentTimeSecondAddr, TimeS);
      InfoPT = GetDelayTime(TimeD, TimeH, TimeM);
      if (InfoPT.Success)
      {
        DelayTime = InfoPT.PresentTime;
      }
    }

    ShowDecimalSystem(0, 12, TimeD);ShowDecimalSystem(5, 12, TimeH);ShowDecimalSystem(15, 12, TimeM);

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
        ++TimeD;
        if(TimeD >= 7){
          TimeD = 0;
        }
      }
    }
    TimeM = TimeML;

    String DormantTime = "休眠到" + GetTimeDayStr(0x01 << TimeD) + "的 ";
    if (TimeH < 10) {
      DormantTime += "0" + String(TimeH);
    } else {
      DormantTime += String(TimeH);
    }
    DormantTime += ":";
    if (TimeM < 10) {
      DormantTime += "0" + String(TimeM);
    } else {
      DormantTime += String(TimeM);
    }

    EEPROM.put(DayAddr, TimeD);
    EEPROM.put(PresentTimeHoursAddr, TimeH);
    EEPROM.put(PresentTimeMinutesAddr, TimeM);
    EEPROM.put(PresentTimeSecondAddr, TimeS);
    EEPROM.commit();
    DEV_Delay_ms(10);
    Debug("唤醒时间：" + GetTimeDayStr(1 << TimeD) + ", " + String(TimeH) + ":" + String(TimeM) + ":" + String(TimeS) + "\n");
    if(InfoPT.Success){
      CN_Show(0, 76, DormantTime.c_str());
    }
  }

  // 刷新屏幕显示内容
  for (int i = 0; i < 5; ++i)
  {
    RenovateScreen(BlackImage);
    DEV_Delay_ms(10);
  }

  if (CaseInfo != Network_Wed)
  {
    // 休眠
    Debug("休眠时间：" + String(DelayTime) + "分\n");
    unsigned long long int SleepTime = ((unsigned long long int)DelayTime) * 60 * 1000000;
    Debug(String(SleepTime) + "us\n");
    Serial.print("总耗时:" + String(millis() - WorkConsumeTime) + "ms\n");
    // 设置唤醒时间
    esp_sleep_enable_timer_wakeup(SleepTime);
    // 进入深度睡眠状态
    esp_deep_sleep_start();
  }
}

bool HasClientVal;
void loop() {

#if Bluetooth
  if (CaseInfo == Network_Wed){
    // SerialBT.hasClient() 是否连接设备
    if(HasClientVal != SerialBT.hasClient()){
      HasClientVal = SerialBT.hasClient();

      if(HasClientVal){
        Serial.print("连接\n");
      }else{
        Serial.print("断开\n");
      }
    }

    // SerialBT.available() 有多少数据可以读
    // 蓝牙 → 串口转发
    if (SerialBT.available()) {
      String receivedData = SerialBT.readString();
      Serial.print("[蓝牙接收] ");
      Serial.println(receivedData);

      Paint_Clear(WHITE);
      CN_Show(34, 0, receivedData.c_str());
      // 刷新屏幕显示内容
      for (int i = 0; i < 5; ++i)
      {
        RenovateScreen(BlackImage);
        DEV_Delay_ms(10);
      }
    }

    // 串口 → 蓝牙转发
    if (Serial.available()) {
      String sendData = Serial.readStringUntil('\n');
      SerialBT.println(sendData);
      Serial.print("[蓝牙发送] ");
      Serial.println(sendData);
    }
    
    DEV_Delay_ms(20);
  }
#endif

}
