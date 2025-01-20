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
#include <WebServer.h>
// #include "WebDAV.h"

const char *ssid = "KT-2.4G", *password = "aurex123456";
// const char* ssid = "CMCC-U55E", *password = "eku7dx5f";
// const char* ssid = "道生", *password = "369784512";
// const char* ssid = "USER_128978", *password = "70438480";

// 创建Web服务器实例，监听端口80
WebServer server(80);



#define DelayMinute 5

UBYTE *BlackImage;



// 将字符串写入EEPROM
void writeStringToEEPROM(int startAddress, String message) {
  int length = message.length();
  
  // 将字符串的每个字符写入EEPROM
  for (int i = 0; i < length; i++) {
    EEPROM.write(startAddress + i, message[i]);
  }
  
  // 在字符串末尾添加一个NULL终止符
  EEPROM.write(startAddress + length, '\0');
  
  EEPROM.commit();  // 提交更改并保存到EEPROM
  Serial.println("数据已写入EEPROM！");
}

// 从EEPROM读取字符串
String readStringFromEEPROM(int startAddress) {
  String message = "";
  char ch = EEPROM.read(startAddress);
  
  // 逐个读取字符直到遇到NULL终止符
  while (ch != '\0') {
    message += ch;
    startAddress++;
    ch = EEPROM.read(startAddress);
  }

  return message;
}


bool ConnectWIFI()
{

  // 读取字符串
  String ssidConfig = readStringFromEEPROM(4);
  String passwordConfig = readStringFromEEPROM(400);

  Debug("链接：");
  Debug(ssidConfig);
  Debug(", ");
  Debug(passwordConfig);
  Debug("\n");

  // 启用WiFi模块
  WiFi.mode(WIFI_STA);
  // 连接WiFi
  WiFi.begin(ssidConfig, passwordConfig);

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






// HTML表单，供用户输入Wi-Fi信息
const char* htmlForm = R"rawliteral(
  <!DOCTYPE HTML><html>
  <head><title>WiFi Configuration</title></head>
  <body>
    <h1>请输入WiFi网络名称和密码</h1>
    <form action="/config" method="POST">
      <label for="ssid">WiFi名称(SSID):</label><br>
      <input type="text" id="ssid" name="ssid" required><br><br>
      <label for="password">WiFi密码:</label><br>
      <input type="password" id="password" name="password" required><br><br>
      <input type="submit" value="连接">
    </form>
  </body>
  </html>
)rawliteral";


// 根路径请求的处理函数
void handleRoot() {
  server.send(200, "text/html", htmlForm);
}


// 处理WiFi配置提交
void handleConfig() {
  String ssidConfig = server.arg("ssid");
  String passwordConfig = server.arg("password");

  Debug(ssidConfig);
  Debug("\n");
  Debug(passwordConfig);


  writeStringToEEPROM(4, ssidConfig);
  writeStringToEEPROM(400, passwordConfig);


  // 读取字符串
  ssidConfig = readStringFromEEPROM(4);
  passwordConfig = readStringFromEEPROM(400);

  // 提示用户已提交WiFi信息
  String response = "<h1>正在连接到WiFi网络...</h1>";
  response += "<p>WiFi名称: " + ssidConfig + "</p>";
  
  server.send(200, "text/html", response);

  // 调用esp_restart()函数进行重启
  esp_restart();
}


/* Entry point ----------------------------------------------------------------*/
void setup()
{
  

  /*
  
  // 初始化 EEPROM
  EEPROM.begin(512);

  
  */

  // 初始化串口
  Serial.begin(115200);
  

  // 初始化 EEPROM
  EEPROM.begin(512);

  


  // 初始化 12引脚口
  pinMode(12, INPUT_PULLUP);
  int I_delay = 300;
  while(--I_delay){
    if(digitalRead(12) == 0){
      Debug("开启设置");
      // 设置ESP32为AP模式并启动，不使用密码
      WiFi.mode(WIFI_MODE_AP);
      WiFi.softAP("ESP32-001");
      Serial.println(WiFi.softAPIP());

      // 定义根路径的回调函数
      server.on("/", handleRoot);
      server.on("/config", HTTP_POST, handleConfig);  // 提交Wi-Fi信息进行连接

      // 启动Web服务器
      server.begin();

      return;
    }
    DEV_Delay_ms(10);
  }


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

void loop() {
  // 处理来自客户端的HTTP请求
  server.handleClient();
}


