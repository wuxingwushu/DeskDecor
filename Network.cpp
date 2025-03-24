#include "Network.h"
#include "EepromString.h"
#include "DEV_Config.h"
#include "Debug.h"
#include <WiFi.h>
#include <AsyncTCP.h>  // ESP32 依赖
#include <SPIFFS.h>
#include <ArduinoJson.h>

// Wed服务器
AsyncWebServer server(80);

NetworkCase ConnectWIFI() {
  // 初始化 12引脚口
  pinMode(12, INPUT_PULLUP);
  // 启用WiFi模块
  WiFi.mode(WIFI_STA);
  if (digitalRead(12) == 0) {
    Debug("Wed服务\n");
    return Network_Wed;
  }

  unsigned char WifiIndex;
  EEPROM.get(WiFiIndexAddr, WifiIndex);
  if (WifiIndex >= WifiDateMaxSize) {
    /* 查询是否有以知WIFI */

    // 异步查询
    WiFi.scanNetworks(true);
    // 获取所以WIFI名字
    String WifiNameS[WifiDateMaxSize];
    Debug("以知WIFI:\n");
    for (unsigned int i = 0; i < WifiDateMaxSize; ++i) {
      WifiNameS[i] = readStringFromEEPROM(WifiNameAddr + (WiFiStrInterval * i));
      Debug(WifiNameS[i] + "\n");
    }

    String WifiName;    // WIFI 名字（临时值）
    int RSSI = -10000;  // 信号强度 （越大越强, 值为 0 是 RSSI 信号最强意思）
    Debug("查询到的WIFI:\n");
    int WiFiSize = -1;      // 查询附近有什么WIFI
    while (WiFiSize < 0) {  // 等待 WIFI 查询结束
      WiFiSize = WiFi.scanComplete();
      if (digitalRead(12) == 0) {
        Debug("Wed服务\n");
        return Network_Wed;
      }
      DEV_Delay_ms(100);
    }
    // 遍历扫描结果，是否有已知WIFI，且选择信号最强的
    for (unsigned int i = 0; i < WiFiSize; ++i) {
      WifiName = WiFi.SSID(i);
      Debug(WifiName + "\n");
      // 信号是否有所增加
      if (WiFi.RSSI(i) > RSSI) {
        // 查询是否有这个WIFI信息
        for (unsigned int k = 0; k < WifiDateMaxSize; ++k) {
          if (WifiNameS[k] == WifiName) {
            // 选择这个WIFI
            WifiIndex = k;
            RSSI = WiFi.RSSI(i);
          }
        }
      }
    }

    if (WifiIndex >= WifiDateMaxSize) {
      ++WifiIndex;
      if (WifiIndex >= (WifiDateMaxSize + 10)) {
        Debug("低频扫描模式\n");
        return LowScanMode;
      }
      // 当没有查到对应WIFI时只判断是否进入Wed模式
      EEPROM.put(WiFiIndexAddr, WifiIndex);
      EEPROM.commit();
      Debug("不存在已知网络！\n");
      return Network_Not;
    } else {
      EEPROM.put(WiFiIndexAddr, WifiIndex);
      EEPROM.commit();
      Debug("查询到WIFI: " + WifiNameS[WifiIndex]);
    }
  }


  /************************/


  // 读取字符串
  String ssidConfig = readStringFromEEPROM(WifiNameAddr + (WiFiStrInterval * WifiIndex));
  String passwordConfig = readStringFromEEPROM(WifiPassAddr + (WiFiStrInterval * WifiIndex));
  Debug("链接：" + ssidConfig + "," + passwordConfig + "\n");

  // 连接WiFi
  WiFi.begin(ssidConfig.c_str(), passwordConfig.c_str());

  unsigned char Count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if (digitalRead(12) == 0) {
      Debug("Wed服务\n");
      return Network_Wed;
    }
    ++Count;
    if (Count > 50) {  // 几次后没法连接判定为没有网络
      EEPROM.put(WiFiIndexAddr, ((unsigned char)WifiDateMaxSize));
      EEPROM.commit();
      Debug("\n连接失败\n");
      return Network_Not;
    }
    DEV_Delay_ms(100);
    Debug(".");
  }
  Debug("\n连接成功\n");
  return Network_Ok;
}

String WebServerFun() {
  // 确保首先断开了STA模式下的任何连接
  WiFi.disconnect(true);

  // 设置ESP32为AP模式并启动，不使用密码
  WiFi.mode(WIFI_AP);
  WiFi.scanNetworks(true);
  WiFi.softAP("一言墨水屏");

  // 响应页面
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", RootHtml);
  });
  server.on("/ttf", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", FileHtml);
  });
  server.on("/wifi", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", WifiHtml);
  });
  server.on("/set", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", SetHtml);
  });

  // 配置wifi信息
  server.on("/wifi/config", HTTP_POST, handleWifiConfig);  // 提交Wi-Fi信息进行连接
  // 获取wifi信息
  server.on("/GetWifiInfo", HTTP_GET, GetWifiInfo);
  // 获取以储存WIFI
  server.on("/GetStoreWifi", HTTP_GET, GetStoreWifi);
  // 获取设置信息
  server.on("/GetSetInfo", HTTP_GET, GetSetInfo);
  // 配置设置
  server.on("/set/config", HTTP_POST, handleSetConfig);
  // 重启
  server.on("/restart", handleRestart);
  // 获取文件列表
  server.on("/files", HTTP_GET, GetFileList);
  // 文件下载
  server.on("/download", HTTP_GET, FileDownload);
  // 文件删除
  server.on("/delete", HTTP_DELETE, FileDeletion);
  // 获取存储信息
  server.on("/storage", HTTP_GET, RetrieveStorageInformation);
  // 文件上传处理
  server.on(
    "/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
      request->send(200);
    },
    FileUploadProcessing);
  // 格式化
  server.on("/FormatFloppyDisk", HTTP_POST, FormatFloppyDisk);

  // 启动Web服务器
  server.begin();

  return WiFi.softAPIP().toString();
}

void GetWifiInfo(AsyncWebServerRequest *request) {
  //Debug("启动异步WiFi扫描\n");
  while (WiFi.scanComplete() < 0) { DEV_Delay_ms(100); };
  int n = WiFi.scanComplete();
  String json = "[";
  for (int i = 0; i < n; ++i) {
    if (i != 0) json += ',';
    json += "{\"name\":\"" + WiFi.SSID(i) + "\"}";
  }
  json += "]";
  Debug(json + "\n");
  request->send(200, "application/json", json);
}

void GetStoreWifi(AsyncWebServerRequest *request) {
  Debug("获取储存wifi\n");
  String json = "[";
  for (unsigned int i = 0; i < WifiDateMaxSize; ++i) {
    if (json != "[")
      json += ',';
    json += "{\"name\":\"" + readStringFromEEPROM(WifiNameAddr + (WiFiStrInterval * i)) + "\",";
    json += "\"size\":" + String(i) + "}";
  }
  json += "]";
  request->send(200, "application/json", json);
}

String TimeStandard(unsigned char H, unsigned char M) {
  String TimeString = "";
  if (H < 10) {
    TimeString += "0" + String(H);
  } else {
    TimeString += String(H);
  }
  TimeString += ":";
  if (M < 10) {
    TimeString += "0" + String(M);
  } else {
    TimeString += String(M);
  }
  return TimeString;
}

void GetSetInfo(AsyncWebServerRequest *request) {
  Debug("获取设置信息\n");
  unsigned short shu;
  EEPROM.get(SleepValueAddr, shu);
  unsigned char WorkDay_, StartHours, StartMinutes, EndHours, EndMinutes, APIPassage;
  EEPROM.get(StartTimeHoursAddr, StartHours);
  EEPROM.get(StartTimeMinutesAddr, StartMinutes);
  EEPROM.get(EndTimeHoursAddr, EndHours);
  EEPROM.get(EndTimeMinutesAddr, EndMinutes);
  EEPROM.get(SentenceAPIPassageAddr, APIPassage);
  float LatitudeVal, LongitudeVal;
  EEPROM.get(LatitudeAddr, LatitudeVal);
  EEPROM.get(LongitudeAddr, LongitudeVal);
  EEPROM.get(WorkDayAddr, WorkDay_);

  String json = "{";
  json += "\"TimeVal\":" + String(shu) + ",";
  json += "\"StartTime\":\"" + TimeStandard(StartHours, StartMinutes) + "\",";
  json += "\"EndTime\":\"" + TimeStandard(EndHours, EndMinutes) + "\",";
  json += "\"Latitude\":" + String(LatitudeVal) + ",";
  json += "\"Longitude\":" + String(LongitudeVal) + ",";
  json += "\"BoolFlage\":" + String(APIPassage) + ",";
  json += "\"WeekdayFlags\":" + String(WorkDay_);
  json += "}";
  Debug(json + "\n");
  request->send(200, "application/json", json);
}

// 处理WiFi配置提交
void handleWifiConfig(AsyncWebServerRequest *request) {
  unsigned int NumberConfig = request->arg("number").toInt();
  String ssidConfig = request->arg("ssid");
  String passwordConfig = request->arg("password");

  if (ssidConfig.length() > (WiFiStrInterval / 2)) {
    Debug("WiFi名字超出设定储存空间\n");
    goto handleWifiConfigEnd;
  }
  if (passwordConfig.length() > (WiFiStrInterval / 2)) {
    Debug("WiFi密码超出设定储存空间\n");
    goto handleWifiConfigEnd;
  }

  Debug("设置wifi\n");
  Debug("位置：" + String(NumberConfig) + "\n");
  Debug("名字：" + ssidConfig + "\n");
  Debug("密码：" + passwordConfig + "\n");

  writeStringToEEPROM(WifiNameAddr + (NumberConfig * WiFiStrInterval), ssidConfig);
  writeStringToEEPROM(WifiPassAddr + (NumberConfig * WiFiStrInterval), passwordConfig);
handleWifiConfigEnd:
  request->send(200, "text/html", RootHtml);
}

// 处理WiFi配置提交
void handleSetConfig(AsyncWebServerRequest *request) {
  String timeConfig = request->arg("TimeVal");
  Debug(timeConfig + "\n");
  String StartTimeConfig = request->arg("StartTime");
  Debug(StartTimeConfig + "\n");
  String EndTimeConfig = request->arg("EndTime");
  Debug(EndTimeConfig + "\n");
  String LatitudeConfig = request->arg("Latitude");
  Debug(LatitudeConfig + "\n");
  String LongitudeConfig = request->arg("Longitude");
  Debug(LongitudeConfig + "\n");
  String BoolFlageConfig = request->arg("BoolFlage");
  Debug(BoolFlageConfig + "\n");
  String WorkDayConfig = request->arg("WeekdayFlags");
  Debug(WorkDayConfig + "\n");

  // 读取字符串
  Debug("储存为:\n");
  int shu = timeConfig.toInt();
  EEPROM.put(SleepValueAddr, shu);
  unsigned char HMData = StartTimeConfig.substring(0, 2).toInt();
  EEPROM.put(StartTimeHoursAddr, HMData);
  Debug(((int)HMData));
  Debug("\n");
  HMData = StartTimeConfig.substring(3, 5).toInt();
  EEPROM.put(StartTimeMinutesAddr, HMData);
  Debug(((int)HMData));
  Debug("\n");
  HMData = EndTimeConfig.substring(0, 2).toInt();
  EEPROM.put(EndTimeHoursAddr, HMData);
  Debug(((int)HMData));
  Debug("\n");
  HMData = EndTimeConfig.substring(3, 5).toInt();
  EEPROM.put(EndTimeMinutesAddr, HMData);
  Debug(((int)HMData));
  Debug("\n");
  float LXXitude = LatitudeConfig.toFloat();
  Debug(LXXitude);
  Debug("\n");
  EEPROM.put(LatitudeAddr, LXXitude);
  LXXitude = LongitudeConfig.toFloat();
  Debug(LXXitude);
  Debug("\n");
  EEPROM.put(LongitudeAddr, LXXitude);
  char timezone_offset = (int)(LXXitude / 15.0 + (fmod(LXXitude, 15.0) >= 7.5 ? 1 : 0));
  Debug((int)timezone_offset);
  Debug("\n");
  EEPROM.put(TimeZoneAddr, timezone_offset);
  HMData = BoolFlageConfig.toInt();
  Debug((int)HMData);
  Debug("\n");
  EEPROM.put(SentenceAPIPassageAddr, HMData);
  HMData = WorkDayConfig.toInt();
  Debug((int)HMData);
  Debug("\n");
  EEPROM.put(WorkDayAddr, HMData);
  EEPROM.put(WiFiIndexAddr, char(WifiDateMaxSize));
  EEPROM.commit();

  request->send(200, "text/html", RootHtml);
}

void handleRestart(AsyncWebServerRequest *request) {
  // 提示用户已提交WiFi信息
  String response = "<h1>重启中...</h1>";
  request->send(200, "text/html", response);
  Debug("重启\n");
  DEV_Delay_ms(100);
  // 调用esp_restart() 函数进行重启
  esp_restart();
}

// 获取文件列表
void GetFileList(AsyncWebServerRequest *request) {
  Debug("获取文件列表\n");
  String json = "[";
  File root = SPIFFS.open("/", "r");
  File file = root.openNextFile();
  while (file) {
    if (json != "[")
      json += ',';
    json += "{\"name\":\"" + String(file.name()) + "\",";
    json += "\"size\":" + String(file.size()) + "}";
    file.close();
    file = root.openNextFile();
  }
  json += "]";
  request->send(200, "application/json", json);
}
// 文件下载
void FileDownload(AsyncWebServerRequest *request) {
  // 安全处理阶段
  String filename = "/" + request->getParam("file")->value();
  Debug("请求下载:" + filename + "\n");

  File file = SPIFFS.open(filename.c_str());  // 打开芯片内存储的文件
  if (!file) {
    Debug("文件未找到:" + filename + "\n");
    request->send(404, "text/plain", "文件未找到");
    return;
  }

  // 创建异步响应对象
  AsyncWebServerResponse *response = request->beginResponse(
    "application/octet-stream",
    file.size(),
    [filename](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
      static File fsFile = SPIFFS.open(filename.c_str(), "r");

      if (!fsFile) {
        Debug("文件打开失败\n");
        return 0;
      }

      if (index == 0) {
        fsFile.seek(0);  // 重置文件指针
      }

      size_t bytesRead = fsFile.read(buffer, maxLen);
      Debug(String(bytesRead) + "\n");

      if (bytesRead == 0) {
        fsFile.close();
        Debug("文件传输完成\n");
      }

      return bytesRead;
    });

  file.close();

  // 设置响应头（正确方法）
  response->addHeader("Content-Disposition", "attachment; filename=\"" + request->getParam("file")->value() + "\"");
  response->addHeader("Cache-Control", "no-cache");

  // 发送响应
  request->send(response);
}
// 文件删除
void FileDeletion(AsyncWebServerRequest *request) {

  if (request->hasParam("file")) {
    String filename = "/" + request->getParam("file")->value();
    Debug("删除：" + filename + "\n");
    if (SPIFFS.remove(filename)) {
      Debug("以删除：" + filename + "\n");
      request->send(200, "text/plain", "File deleted");
    } else {
      Debug("不存在：" + filename + "\n");
      request->send(500, "text/plain", "Delete failed");
    }
  } else {
    Debug("删除错误！\n");
    request->send(400, "text/plain", "Bad request");
  }
}
// 获取存储信息
void RetrieveStorageInformation(AsyncWebServerRequest *request) {
  Debug("获取储存信息\n");
  size_t total = SPIFFS.totalBytes();
  size_t used = SPIFFS.usedBytes();
  String json = "{";
  json += "\"total\":" + String(total) + ",";
  json += "\"used\":" + String(used) + ",";
  json += "\"free\":" + String(total - used);
  json += "}";
  Debug(json + "\n");
  request->send(200, "application/json", json);
}
// 文件上传处理
void FileUploadProcessing(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (!index) {
    filename = "/" + filename;
    Debug("打开：" + filename + "\n");
    request->_tempFile = SPIFFS.open(filename, "w");
  }
  if (request->_tempFile) {
    request->_tempFile.write(data, len);
  }
  if (final) {
    if (request->_tempFile) {
      Debug("关闭：" + filename + "\n");
      request->_tempFile.close();
    }
  }
}

// 添加全局变量
TaskHandle_t formatTaskHandle;

// 独立格式化任务
void formatTask(void *pvParams) {
  AsyncWebServerRequest *request = (AsyncWebServerRequest*)pvParams;
  disableLoopWDT();  // 仅需禁用Loop看门狗
  
  Debug("开始格式化...\n");
  bool success = SPIFFS.format();
  
  if(success) {
    Debug("格式化成功，尝试重新挂载\n");
    request->send(200);
  } else {
    Debug("格式化失败\n");
    request->send(500, "text/plain", "Format Failed");
  }
  
  enableLoopWDT();
  vTaskDelete(formatTaskHandle); // 删除当前任务
}

void FormatFloppyDisk(AsyncWebServerRequest *request) {
  // 创建独立任务执行格式化
  xTaskCreatePinnedToCore(
    formatTask,        // 任务函数
    "FormatTask",      // 任务名称
    4096,              // 堆栈大小
    (void*)request,    // 参数传递
    2,                 // 优先级（高于loopTask）
    &formatTaskHandle, // 任务句柄
    1                  // 运行在Core1
  );
}

const char *RootHtml = R"rawliteral(
<!DOCTYPE html>
<html lang="zh">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, viewport-fit=cover">
    <title>设备控制中心</title>
    <style>
        /* 新增 确保根元素和body的背景色统一 */
        html, body {
            margin: 0;
            padding: 0;
            background-color: #24292e; /* 与容器颜色一致 */
            overflow-x: hidden; /* 防止横向滚动 */
            overflow-y: hidden; /* 防止横向滚动 */
        }

        :root {
            --icon-size: 30vmin;
            --gap-size: 5vmin;
        }

        .container {
            display: flex;
            flex-wrap: wrap;
            gap: var(--gap-size);
            justify-content: center;
            align-content: center; /* 关键修改 */
            align-items: center; /* 改为align-items */
            min-height: 100dvh;
            padding: 5vmin;
            width: 100%; /* 添加宽度限制 */
            height: 100vh; /* 新增高度定义 */
            box-sizing: border-box; /* 包含padding */
        }

        .icon-link {
            flex: 0 0 auto; /* 防止flex伸缩 */
            width: calc(33.333% - var(--gap-size)*2/3); /* 三列精确计算 */
            max-width: 300px; /* 最大尺寸限制 */
            aspect-ratio: 1/1; /* 保持方形 */
            padding: 12px;
            transition: transform 0.3s cubic-bezier(0.25, 0.46, 0.45, 0.94);
        }

        .icon {
            width: 100%;
            height: 100%;
            aspect-ratio: 1/1;
        }

        @media (orientation: portrait) {
            .icon-link {
                width: calc(50% - var(--gap-size)/2); /* 两列精确计算 */
            }
        }

        @media (min-width: 768px) {
            .icon-link {
                width: calc(25% - var(--gap-size)*3/4); /* 四列精确计算 */
            }
        }

        @media (orientation: landscape) {
            .container {
                flex-direction: row;
                padding: 5vmax;
            }
        }

        .icon-link:hover {
            transform: scale(1.08);
            /* 移除 rotate 保持纯缩放效果 */
        }
        .icon-link:active {
            transform: scale(0.95);
        }

        @media (prefers-color-scheme: dark) {
            .icon {
                filter: invert(1) hue-rotate(180deg) brightness(1.2);
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <!-- WiFi图标 -->
        <a href="/wifi" class="icon-link">
            <svg t="1737446876153" class="icon" viewBox="0 0 1122 1024" version="1.1" xmlns="http://www.w3.org/2000/svg"
                p-id="2433" width="200" height="200">
                <path
                    d="M561.152 910.336c-7.68 0-25.088-13.824-53.248-41.984-27.648-28.16-41.472-45.568-41.472-53.248 0-12.288 11.776-22.528 35.84-30.72s43.52-12.8 59.392-12.8c15.872 0 35.328 4.096 59.392 12.8s35.84 18.432 35.84 30.72c0 7.68-13.824 25.6-41.472 53.248-29.184 28.16-46.592 41.984-54.272 41.984z"
                    fill="#1296DB" p-id="2434"></path>
                <path
                    d="M715.264 755.712c-0.512 0-8.192-4.608-23.04-14.336-14.336-9.728-33.792-18.944-57.856-28.672-24.064-9.728-48.64-14.336-73.216-14.336-24.576 0-49.152 4.608-73.216 14.336-24.064 9.728-43.52 18.944-57.856 28.672-14.336 9.728-22.016 14.336-23.04 14.336-6.656 0-24.576-14.336-53.248-43.008s-43.008-46.08-43.008-53.248c0-5.12 2.048-9.216 5.632-13.312 29.696-29.184 67.072-52.224 112.128-69.12s89.088-25.088 133.12-25.088c44.032 0 88.064 8.192 133.12 25.088 45.056 16.896 82.432 39.936 112.128 69.12 3.584 3.584 5.632 8.192 5.632 13.312 0 6.656-14.336 24.576-43.008 53.248-29.696 28.672-47.104 43.008-54.272 43.008z"
                    fill="#19A3E0" p-id="2435"></path>
                <path
                    d="M871.424 600.064c-4.096 0-8.704-1.536-13.312-4.608-51.712-39.936-99.84-69.632-143.872-88.064s-95.232-28.16-153.088-28.16c-32.256 0-65.024 4.096-97.28 12.8S402.944 510.464 378.88 522.24c-24.064 11.776-45.568 23.552-65.024 35.328s-34.304 22.016-45.056 30.208c-11.264 8.192-16.896 12.8-17.92 12.8-6.656 0-24.064-14.336-52.736-43.008s-43.008-46.08-43.008-53.248c0-4.608 2.048-8.704 5.632-12.8 50.176-50.176 111.104-89.088 182.784-117.248 71.68-27.648 143.872-41.472 217.088-41.472s145.408 13.824 217.088 41.472c71.68 27.648 132.608 67.072 182.784 117.248 3.584 3.584 5.632 8.192 5.632 12.8 0 6.656-14.336 24.576-43.008 53.248s-45.056 42.496-51.712 42.496z"
                    fill="#48B5E5" p-id="2436"></path>
                <path
                    d="M1026.048 445.44c-4.096 0-8.192-1.536-12.8-5.12-68.096-59.904-138.752-104.96-212.48-135.168-73.216-30.208-153.6-45.568-240.128-45.568-87.04 0-166.912 15.36-240.128 45.568S176.64 380.416 108.544 440.32c-4.096 3.584-8.192 5.12-12.8 5.12-6.656 0-24.064-14.336-52.736-43.008S0 356.352 0 349.184c0-5.12 2.048-9.216 5.632-13.312C76.8 265.216 161.792 210.432 260.096 171.52s198.656-58.368 301.056-58.368 202.752 19.456 301.056 58.368 183.296 93.696 254.464 164.352c3.584 3.584 5.632 8.192 5.632 13.312 0 6.656-14.336 24.576-43.008 53.248-28.672 28.672-46.592 43.008-53.248 43.008z"
                    fill="#80D8FF" p-id="2437"></path>
            </svg>
        </a>

        <!-- 设置图标 -->
        <a href="/set" class="icon-link">
            <svg t="1737633885234" class="icon" viewBox="0 0 1024 1024" version="1.1" xmlns="http://www.w3.org/2000/svg"
                p-id="7193" width="200" height="200">
                <path
                    d="M951.68 428.8a23.808 23.808 0 0 0-0.64-4.224v-0.768l-0.448-2.24c-7.04-34.56-30.016-56.896-58.496-56.896h-4.672c-48.64 0-88-39.552-88-88 0-11.2 5.12-27.072 7.36-32.64 13.824-32.256-0.896-68.928-35.008-87.68l-107.264-60.672-1.984-0.64c-8.064-2.624-17.28-5.76-27.712-5.76-19.392 0-41.216 8.96-54.72 22.528-16.896 16.64-51.2 41.6-71.616 41.6-20.288 0-54.656-24.832-71.616-41.6a78.528 78.528 0 0 0-54.656-22.528c-10.688 0-19.712 3.008-27.712 5.76l-1.792 0.64-112.512 60.928-0.64 0.384c-27.328 17.088-38.4 56.32-24.576 87.424l0.192 0.384 0.256 0.384c2.176 4.928 8.96 21.504 8.96 36.032 0 48.64-39.616 88-88 88h-4.672c-29.824 0-52.096 21.952-58.496 57.28l-0.448 1.984v0.704c0 1.024-0.384 2.432-0.64 4.224-2.56 15.104-8.512 50.688-8.512 79.808 0 29.056 5.888 64.64 8.448 79.808a24 24 0 0 0 0.704 4.16v0.832l0.448 2.176c7.04 34.56 29.952 56.96 58.496 56.96h2.368c48.64 0 88 39.552 88 87.936 0 11.2-5.184 27.136-7.36 32.704-13.312 30.272-0.704 69.184 28.672 88.832l0.832 0.384 105.984 59.008 1.984 0.64c8 2.624 17.088 5.76 27.52 5.76 22.208 0 42.24-8.512 54.656-22.528 1.28-0.896 2.432-2.112 3.84-3.264 12.8-11.2 47.168-40.832 69.888-40.832 16.896 0 45.184 17.728 73.728 46.208 14.4 14.208 34.24 22.464 54.656 22.464 13.824 0 24-3.776 35.584-9.472l0.448-0.192 108.672-60.16 0.384-0.32c27.328-17.152 38.4-56.32 24.512-87.424l-0.192-0.384-0.192-0.384c-0.192-0.128-8.704-17.856-7.104-33.728l0.192-1.024v-0.96c0-48.64 39.616-88 88-88h4.992c29.824 0 52.096-22.016 58.496-57.344l0.448-1.92v-0.768l0.64-3.584c2.624-14.72 8.64-49.024 8.64-80.384 0.192-28.992-5.76-64.512-8.32-79.616z m-440 222.4a139.2 139.2 0 1 1 0-278.4 139.2 139.2 0 0 1 0 278.4z"
                    fill="#00BAAD" p-id="7194"></path>
            </svg> </a>

        <!-- 字体图标 -->
        <a href="/ttf" class="icon-link">
            <svg t="1740643589326" class="icon" viewBox="0 0 1024 1024" version="1.1" xmlns="http://www.w3.org/2000/svg"
                p-id="3754" width="200" height="200">
                <path
                    d="M920 416H616c-4.4 0-8 3.6-8 8v112c0 4.4 3.6 8 8 8h48c4.4 0 8-3.6 8-8v-56h60v320h-46c-4.4 0-8 3.6-8 8v48c0 4.4 3.6 8 8 8h164c4.4 0 8-3.6 8-8v-48c0-4.4-3.6-8-8-8h-46V480h60v56c0 4.4 3.6 8 8 8h48c4.4 0 8-3.6 8-8V424c0-4.4-3.6-8-8-8zM656 296V168c0-4.4-3.6-8-8-8H104c-4.4 0-8 3.6-8 8v128c0 4.4 3.6 8 8 8h56c4.4 0 8-3.6 8-8v-64h168v560h-92c-4.4 0-8 3.6-8 8v56c0 4.4 3.6 8 8 8h264c4.4 0 8-3.6 8-8v-56c0-4.4-3.6-8-8-8h-92V232h168v64c0 4.4 3.6 8 8 8h56c4.4 0 8-3.6 8-8z"
                    fill="#1296DB" p-id="3755"></path>
            </svg></a>

        <!-- 重启图标 -->
        <a href="/restart" class="icon-link">
            <svg t="1737633375421" class="icon" viewBox="0 0 1024 1024" version="1.1" xmlns="http://www.w3.org/2000/svg"
                p-id="1499" width="200" height="200">
                <path
                    d="M512 0C229.239467 0 0 229.239467 0 512c0 282.760533 229.239467 512 512 512 282.760533 0 512-229.239467 512-512C1024 229.239467 794.760533 0 512 0z m0 786.005333c-141.380267 0-256-114.619733-256-256 0-141.380267 114.619733-256 256-256V200.874667l146.295467 109.704533L512 420.317867v-73.130667a182.869333 182.869333 0 1 0 182.869333 182.818133c0-43.434667 73.130667-44.970667 73.130667 0 0 141.380267-114.619733 256-256 256z"
                    fill="#3498DA" p-id="1500"></path>
            </svg> </a>
    </div>
</body>
</html>
)rawliteral";

const char *FileHtml = R"rawliteral(
<!DOCTYPE html>
<html>

<head>
    <meta charset="UTF-8">
    <title>ESP32 文件管理器</title>
    <style>
        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Helvetica, Arial, sans-serif;
            margin: 20px;
            background-color: #0d1117;
            color: #c9d1d9;
            line-height: 1.5;
        }

        .container {
            max-width: 800px;
            margin: 0 auto;
        }

        .section {
            margin-bottom: 20px;
            padding: 20px;
            background-color: #161b22;
            border: 1px solid #30363d;
            border-radius: 6px;
            box-shadow: 0 1px 0 rgba(48, 54, 61, 0.5);
        }

        h1,
        h2 {
            color: #e6edf3;
            border-bottom: 1px solid #30363d;
            padding-bottom: 0.3em;
            margin-top: 0;
        }

        h1 {
            font-size: 24px;
        }

        h2 {
            font-size: 20px;
        }

        /* 自定义文件上传按钮 */
        .custom-file-upload {
            display: inline-block;
            padding: 5px 16px;
            background-color: #21262d;
            border: 1px solid #363b42;
            border-radius: 6px;
            cursor: pointer;
            transition: all 0.1s cubic-bezier(0.3, 0, 0.5, 1);
        }

        .custom-file-info {
            display: inline-block;
        }

        .custom-file-upload:hover {
            background-color: #2d333b;
            border-color: #8b949e;
        }

        input[type="file"] {
            display: none;
        }

        /* 统一按钮样式 */
        button {
            background-color: #21262d;
            border: 1px solid #363b42;
            color: #c9d1d9;
            padding: 5px 16px;
            border-radius: 6px;
            font-size: 14px;
            cursor: pointer;
            transition: all 0.1s cubic-bezier(0.3, 0, 0.5, 1);
        }

        button:hover {
            background-color: #2d333b;
            border-color: #8b949e;
        }

        button:active {
            background-color: #3b424b;
        }

        /* 文件列表样式 */
        ul {
            padding-left: 0;
            margin: 15px 0;
            border: 1px solid #30363d;
            border-radius: 6px;
        }

        li {
            padding: 8px 16px;
            display: flex;
            align-items: center;
            border-bottom: 1px solid #30363d;
        }

        li:last-child {
            border-bottom: none;
        }

        li::before {
            content: "📄";
            margin-right: 12px;
            filter: hue-rotate(180deg);
        }

        /* 存储信息样式 */
        #storage {
            padding: 12px;
            background-color: #0d1117;
            border: 1px solid #30363d;
            border-radius: 6px;
            color: #8b949e;
            font-family: ui-monospace, SFMono-Regular, SF Mono, Menlo, Consolas, Liberation Mono, monospace;
        }

        /* 进度条动画 */
        #progressBar {
            transition: width 0.3s ease, background-color 0.3s ease;
        }

        /* 错误状态 */
        .error-progress {
            background-color: #da3633 !important;
        }
    </style>
</head>

<body>
    <div class="container">
        <h1>ESP32 文件管理器</h1>

        <div class="section">
            <h2 class="custom-file-info">存储信息</h2><button onclick="FormatFloppyDisk()">格式化</button>
            <div id="storage">正在加载...</div>
        </div>

        <div class="section">
            <h2>文件上传</h2>
            <label class="custom-file-upload">
                <span>选择文件</span>
                <input type="file" id="fileInput">
            </label>
            <button onclick="uploadFile()">开始上传</button>
            <div id="progressContainer" style="margin-top:15px; display:none;">
                <div style="display:flex; align-items:center; gap:10px;">
                    <div style="width:200px; height:8px; background:#30363d; border-radius:4px;">
                        <div id="progressBar"
                            style="width:0%; height:100%; background:#2ea043; border-radius:4px; transition:width 0.3s ease;">
                        </div>
                    </div>
                    <span id="progressText">0%</span>
                </div>
            </div>
        </div>

        <div class="section">
            <h2>文件列表</h2>
            <button onclick="refreshFiles()" style="margin-bottom: 15px;">🔄 刷新列表</button>
            <div id="fileList"></div>
        </div>
    </div>

    <script>
        // 格式化
        function FormatFloppyDisk() {
            if (!confirm("⚠️ 即将格式化磁盘！\n\n所有文件将被永久删除且不可恢复！\n确定要继续吗？")) {
                return;
            }

            // 显示操作状态
            const storageDiv = document.getElementById('storage');
            storageDiv.innerHTML = '<span style="color:#8b949e">正在格式化...</span>';

            fetch('/FormatFloppyDisk', {
                method: 'POST'
            })
                .then(response => {
                    if (response.ok) {
                        // 刷新界面数据
                        refreshFiles();

                        // 显示成功状态
                        storageDiv.innerHTML = '<span style="color:#2ea043">格式化成功！</span>';
                        setTimeout(() => updateStorage(), 2000); // 2秒后恢复显示
                    } else {
                        throw new Error(`HTTP 错误 ${response.status}`);
                    }
                })
                .catch(error => {
                    console.error('格式化错误:', error);
                    storageDiv.innerHTML = '<span style="color:#da3633">格式化失败！</span>';
                    setTimeout(() => updateStorage(), 2000);
                });
        }

        // 存储信息更新
        function updateStorage() {
            fetch('/storage')
                .then(response => response.json())
                .then(data => {
                    const total = (data.total / 1024).toFixed(2);
                    const used = (data.used / 1024).toFixed(2);
                    const free = (data.free / 1024).toFixed(2);
                    document.getElementById('storage').innerHTML =
                        `总空间: ${total} KB\n已使用: ${used} KB\n剩余空间: ${free} KB`.replace(/\n/g, '<br>');
                });
        }

        // 文件列表刷新
        function refreshFiles() {
            fetch('/files')
                .then(response => response.json())
                .then(files => {
                    const list = files.map(file => `
                        <li>
                            <span style="flex-grow:1">${file.name}</span>
                            <span style="color:#8b949e; margin:0 12px;">${file.size / 1024} KB</span>
                            <button onclick="downloadFile('${file.name}')">⬇️ 下载</button>
                            <button onclick="deleteFile('${file.name}')">🗑️ 删除</button>
                        </li>
                    `).join('');
                    document.getElementById('fileList').innerHTML = `<ul>${list}</ul>`;
                });
        }

        function handleUploadError() {
            const progressBar = document.getElementById('progressBar');
            const progressText = document.getElementById('progressText');
            progressBar.style.width = '100%';
            progressBar.style.backgroundColor = '#da3633';
            progressText.textContent = '上传失败!';
            setTimeout(() => document.getElementById('progressContainer').style.display = 'none', 2000);
        }

        // 文件上传逻辑
        function uploadFile() {
            const fileInput = document.getElementById('fileInput');
            if (!fileInput.files[0]) return alert('请先选择文件');

            // 显示进度容器
            const progressContainer = document.getElementById('progressContainer');
            const progressBar = document.getElementById('progressBar');
            const progressText = document.getElementById('progressText');
            progressContainer.style.display = 'block';

            const xhr = new XMLHttpRequest();
            const formData = new FormData();
            formData.append('file', fileInput.files[0]);

            // 进度事件监听
            xhr.upload.addEventListener('progress', function (e) {
                if (e.lengthComputable) {
                    const percent = (e.loaded / e.total * 100).toFixed(1);
                    progressBar.style.width = percent + '%';
                    progressText.textContent = percent + '%';
                }
            });

            // 完成处理
            xhr.onload = function () {
                if (xhr.status === 200) {
                    progressBar.style.backgroundColor = '#2ea043';
                    progressText.textContent = '上传完成!';
                    setTimeout(() => progressContainer.style.display = 'none', 2000);
                    fileInput.value = '';
                    refreshFiles();
                    updateStorage();
                } else {
                    handleUploadError();
                }
            };

            // 错误处理
            xhr.onerror = function () {
                handleUploadError();
            };

            xhr.open('POST', '/upload');
            xhr.send(formData);
        }



        // 文件删除确认
        function deleteFile(filename) {
            if (confirm(`确定要永久删除 "${filename}" 吗？`)) {
                fetch(`/delete?file=${filename}`, { method: 'DELETE' })
                    .then(response => {
                        if (response.ok) {
                            refreshFiles();
                            updateStorage();
                            alert('🗑️ 文件已删除');
                        }
                    });
            }
        }

        // 增强版下载函数（支持大文件进度跟踪）
        // 前端JavaScript文件下载函数
        function downloadFile(filename) {
            // 1. 创建隐藏的下载链接
            const link = document.createElement('a');

            // 2. 构建下载请求地址
            link.href = `/download?file=${encodeURIComponent(filename)}`;

            // 3. 设置下载属性（强制下载而非预览）
            link.download = filename;

            // 4. 设置链接显示样式
            link.style.display = 'none';

            // 5. 将链接添加到DOM树
            document.body.appendChild(link);

            // 6. 模拟用户点击触发下载
            link.click();

            // 7. 清理临时创建的DOM元素
            document.body.removeChild(link);

            // 8. 错误处理（可选）
            link.onerror = function () {
                console.error('文件下载失败:', filename);
                alert('下载失败，请检查文件名是否正确');
            }
        }

        // 初始化加载
        updateStorage();
        refreshFiles();
    </script>
</body>

</html>
)rawliteral";

const char *SetHtml = R"rawliteral(
<!DOCTYPE HTML>
<html>

<head>
  <meta http-equiv="Content-Type" content="text/html; width=device-width, initial-scale=1.0,charset=utf-8" />
  <title>ESP32 设置界面</title>
  <style>
    body {
      height: 100vh;
      margin: 0;
      font-family: 'SF Mono', 'Roboto Mono', monospace;
      display: flex;
      justify-content: center;
      align-items: center;
      background-color: #24292e;
      /* 暗色背景 */
      color: #c9d1d9;
      /* 浅色文字 */
    }

    .form-input {
      width: calc(100% - 20px);
      padding: 10px;
      margin: 10px 0;
      border: 1px solid #30363d;
      border-radius: 6px;
      box-sizing: border-box;
      background-color: #161b22;
      color: #c9d1d9;
      transition: border-color 0.3s ease;
    }

    .form-input:focus {
      border-color: #58a6ff;
    }

    .submit-button {
      width: 100%;
      padding: 10px;
      margin-top: 10px;
      background-color: #2ea043;
      color: white;
      border: none;
      border-radius: 6px;
      cursor: pointer;
      transition: background-color 0.3s ease;
    }

    .submit-button:hover {
      background-color: #279f42;
    }

    .form-container {
      padding: 20px;
      background-color: #161b22;
      border-radius: 6px;
      box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
    }

    .switch-container {
      display: grid;
      grid-template-columns: repeat(2, 1fr);
      gap: 10px;
      margin: 15px 0;
    }

    .switch-label {
      display: flex;
      align-items: center;
      font-size: 0.9em;
    }

    .switch-label input {
      margin-right: 8px;
    }

    .geo-button {
      width: 100%;
      padding: 8px;
      margin: 10px 0;
      background-color: #2ea043;
      color: white;
      border: none;
      border-radius: 6px;
      cursor: pointer;
      transition: background-color 0.3s ease;
      font-size: 0.9em;
    }

    .geo-button:hover {
      background-color: #279f42;
    }

    .geo-container {
      margin: 15px 0;
    }
  </style>
</head>

<body>
  <form action="/set/config" method="POST" id="passwordForm" class="form-container">
    <p>更新時間(分钟):</p>
    <input type="number" id="TimeVal" name="TimeVal" class="form-input" value="5">

    <p>（开始时间 等于 结束时间 代表不停止工作）</p>
    <p>开始时间:</p>
    <input type="time" id="StartTime" name="StartTime" class="form-input" required value="08:00"
      pattern="[0-2][0-9]:[0-5][0-9]" step="60" placeholder="例如: 08:00">

    <p>结束时间:</p>
    <input type="time" id="EndTime" name="EndTime" class="form-input" required value="17:30"
      pattern="[0-2][0-9]:[0-5][0-9]" step="60" placeholder="例如: 17:30">

    <div class="geo-container">
      <p>位置信息:</p>
      <button type="button" class="geo-button" onclick="getLocation()">自动获取当前位置</button>
    </div>

    <p>纬度:</p>
    <input type="number" id="Latitude" name="Latitude" class="form-input" value="22.9882" step="0.0001" min="-90"
      max="90" placeholder="例如: 22.9882">

    <p>经度:</p>
    <input type="number" id="Longitude" name="Longitude" class="form-input" value="114.3198" step="0.0001" min="-180"
      max="180" placeholder="例如: 114.3198">

    <p>功能开关:</p>
    <div class="switch-container">
      <label class="switch-label">
        <input type="checkbox" class="bool-switch" data-bit="0" checked>
        循环模式
      </label>
      <label class="switch-label">
        <input type="checkbox" class="bool-switch" data-bit="1" checked>
        一言
      </label>
      <label class="switch-label">
        <input type="checkbox" class="bool-switch" data-bit="2" checked>
        ONE
      </label>
      <label class="switch-label">
        <input type="checkbox" class="bool-switch" data-bit="3" checked>
        青桔
      </label>
    </div>
    <input type="hidden" id="BoolFlage" name="BoolFlage">

    <p>工作日选择（多选）:</p>
    <div class="switch-container" style="grid-template-columns: repeat(3, 1fr)">
      <label class="switch-label">
        <input type="checkbox" class="weekday-switch" data-bit="0" checked>
        周日
      </label>
      <label class="switch-label">
        <input type="checkbox" class="weekday-switch" data-bit="1" checked>
        周一
      </label>
      <label class="switch-label">
        <input type="checkbox" class="weekday-switch" data-bit="2" checked>
        周二
      </label>
      <label class="switch-label">
        <input type="checkbox" class="weekday-switch" data-bit="3" checked>
        周三
      </label>
      <label class="switch-label">
        <input type="checkbox" class="weekday-switch" data-bit="4" checked>
        周四
      </label>
      <label class="switch-label">
        <input type="checkbox" class="weekday-switch" data-bit="5" checked>
        周五
      </label>
      <label class="switch-label">
        <input type="checkbox" class="weekday-switch" data-bit="6" checked>
        周六
      </label>
    </div>
    <input type="hidden" id="WeekdayFlags" name="WeekdayFlags">

    <button type="submit" class="submit-button">修改</button>
  </form>
  <script>
    function updateStorage() {
      fetch('/GetSetInfo')
        .then(response => response.json())
        .then(data => {
          document.getElementById('TimeVal').value = data.TimeVal;
          document.getElementById('StartTime').value = data.StartTime;
          document.getElementById('EndTime').value = data.EndTime;
          document.getElementById('Latitude').value = data.Latitude;
          document.getElementById('Longitude').value = data.Longitude;
          document.getElementById('BoolFlage').value = data.BoolFlage;
          document.getElementById('WeekdayFlags').value = data.WeekdayFlags;

          // 更新复选框状态
          const flags = parseInt(data.BoolFlage, 10); // 确保转换为整数
          document.querySelectorAll('.bool-switch').forEach(checkbox => {
            const bit = parseInt(checkbox.dataset.bit); // 获取data-bit值
            checkbox.checked = (flags & (1 << bit)) !== 0; // 检查对应位是否设置
          });

          // 更新星期选择状态
          const weekdayFlags = parseInt(data.WeekdayFlags, 10);
          document.querySelectorAll('.weekday-switch').forEach(checkbox => {
            const bit = parseInt(checkbox.dataset.bit);
            checkbox.checked = (weekdayFlags & (1 << bit)) !== 0;
          });
        });
    }

    document.getElementById('passwordForm').addEventListener('submit', function (e) {
      const loopModeCheckbox = document.querySelector('.bool-switch[data-bit="0"]');
      const otherCheckboxes = document.querySelectorAll('.bool-switch:not([data-bit="0"])');

      // 验证循环模式关闭时其他选项只能选一个
      if (!loopModeCheckbox.checked) {
        const checkedCount = Array.from(otherCheckboxes).filter(cb => cb.checked).length;
        if (checkedCount > 1) {
          alert('当“循环模式”关闭时，只能选择“青桔”、“ONE”或“一言”中的一个！');
          e.preventDefault();
          return;
        }
      }

      // 原始flags计算逻辑
      let flags = 0;
      document.querySelectorAll('.bool-switch').forEach(checkbox => {
        if (checkbox.checked) {
          const bit = parseInt(checkbox.dataset.bit);
          flags |= (1 << bit);
        }
      });
      document.getElementById('BoolFlage').value = flags;


      // 生成星期标志位
      let weekdayFlags = 0;
      document.querySelectorAll('.weekday-switch').forEach(checkbox => {
        if (checkbox.checked) {
          const bit = parseInt(checkbox.dataset.bit);
          weekdayFlags |= (1 << bit);
        }
      });
      document.getElementById('WeekdayFlags').value = weekdayFlags;
    });

    // 实时交互逻辑
    const loopModeCheckbox = document.querySelector('.bool-switch[data-bit="0"]');
    const otherCheckboxes = document.querySelectorAll('.bool-switch[data-bit="1"], .bool-switch[data-bit="2"], .bool-switch[data-bit="3"]');

    // 当循环模式状态改变时
    loopModeCheckbox.addEventListener('change', function () {
      if (!this.checked) {
        // 关闭循环模式时，检查其他选项选择数量
        const checked = Array.from(otherCheckboxes).filter(cb => cb.checked);
        if (checked.length > 1) {
          // 保留最后一个选中的，取消其他
          checked.slice(0, -1).forEach(cb => cb.checked = false);
        }
      }
    });

    // 当其他选项改变时
    otherCheckboxes.forEach(checkbox => {
      checkbox.addEventListener('change', function () {
        if (!loopModeCheckbox.checked && this.checked) {
          // 如果循环模式关闭且当前被选中，取消其他选项
          otherCheckboxes.forEach(other => {
            if (other !== this) other.checked = false;
          });
        }
      });
    });

    // 新增定位功能
    function getLocation() {
      if (navigator.geolocation) {
        navigator.geolocation.getCurrentPosition(
          position => {
            const lat = position.coords.latitude.toFixed(4);
            const lng = position.coords.longitude.toFixed(4);
            document.getElementById('Latitude').value = lat;
            document.getElementById('Longitude').value = lng;
          },
          error => {
            let message = "无法获取位置信息：";
            switch (error.code) {
              case error.PERMISSION_DENIED:
                message += "用户拒绝了位置请求";
                break;
              case error.POSITION_UNAVAILABLE:
                message += "位置信息不可用";
                break;
              case error.TIMEOUT:
                message += "获取位置超时";
                break;
              default:
                message += "未知错误";
            }
            alert(message);
          },
          {
            enableHighAccuracy: true,
            timeout: 5000,
            maximumAge: 0
          }
        );
      } else {
        alert("您的浏览器不支持地理定位功能");
      }
    }

    updateStorage();
  </script>
</body>

</html>
)rawliteral";

const char *WifiHtml = R"rawliteral(
<!DOCTYPE HTML>
<html>

<head>
  <meta http-equiv="Content-Type" content="text/html; width=device-width, initial-scale=1.0,charset=utf-8" />
  <title>WiFi 配置界面</title>
  <style>
    /* 新增数字选择框样式 */
    #numberSelect {
      margin-top: 15px;
      display: none;
    }

    #numberSelect.show {
      display: block;
    }

    .number-select {
      margin: 15px 0;
      display: flex;
      flex-direction: column;
      justify-content: space-between;
    }

    .number-option {
      flex: 1;
      margin: 0 2px;
    }

    .number-option input[type="radio"] {
      display: none;
    }

    .number-option label {
      display: block;
      padding: 8px;
      background: #2ea043;
      border-color: #279f42;
      border: 1px solid #30363d;
      color: #c9d1d9;
      border-radius: 4px;
      text-align: center;
      cursor: pointer;
      transition: all 0.3s ease;
    }

    .number-option input[type="radio"]:checked+label {
      background: #2ea043;
      border-color: #279f42;
    }

    /* 原有样式保持不变 */
    body,
    html {
      height: 100vh;
      margin: 0;
      font-family: 'SF Mono', 'Roboto Mono', monospace;
      display: flex;
      justify-content: center;
      align-items: center;
      background-color: #24292e;
      color: #c9d1d9;
    }

    .wifi-container {
      width: 100vh;
      max-width: 50vh;
      background-color: #161b22;
      border-radius: 6px;
      overflow: hidden;
      box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
    }

    .wifi-item {
      padding: 5vh;
      width: 100%;
      transition: background-color 0.3s ease;
      cursor: pointer;
      border-bottom: 1px solid #30363d;
      text-align: center;
    }

    .wifi-item:hover {
      background-color: #21262d;
    }

    .hidden {
      display: none;
    }

    .form-input {
      width: calc(100% - 20px);
      padding: 10px;
      margin: 10px 0;
      border: 1px solid #30363d;
      border-radius: 6px;
      box-sizing: border-box;
      background-color: #161b22;
      color: #c9d1d9;
      transition: border-color 0.3s ease;
    }

    .form-input:focus {
      border-color: #58a6ff;
    }

    .submit-button {
      width: 100%;
      padding: 10px;
      margin-top: 10px;
      background-color: #2ea043;
      color: white;
      border: none;
      border-radius: 6px;
      cursor: pointer;
      transition: background-color 0.3s ease;
    }

    .submit-button:hover {
      background-color: #279f42;
    }

    .form-container {
      padding: 20px;
      background-color: #161b22;
      border-radius: 6px;
      box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
    }

    label {
      display: block;
      margin-bottom: 5px;
      color: #8b949e;
    }
  </style>
</head>

<body>
  <div id="liebiao" class="wifi-container">
  </div>

  <form action="/wifi/config" method="POST" id="passwordForm" class="form-container hidden">
    <label for="ssid" id="WifiName">WiFi名称:</label>
    <input type="text" id="ssid" placeholder="请输入WiFi名称" name="ssid" class="form-input" required>
    <label for="password" id="WifiPassword">WiFi密码:</label>
    <input type="password" id="password" placeholder="请输入WiFi密码" name="password" class="form-input" required>

    <!-- 数字选择区域 -->
    <div id="numberSelect">
      <label>选择被覆盖WiFi:</label>
      <div class="number-select" id="StoreWifi">
      </div>
    </div>

    <button type="button" id="ButtonJoin" class="submit-button" onclick="handleConnect()">连接</button>
    <button type="button" id="ButtonReturn" class="submit-button" onclick="ReturnInterface()">返回</button>
  </form>
</body>
<script>
  // 获取储存了那些wifi
  function GetStoreWifi() {
    fetch('/GetStoreWifi')
      .then(response => response.json())
      .then(files => {
        const list = files.map(file => `
                        <div class="number-option">
                          <input type="radio" id="num${file.size}" name="number" value="${file.size}" onchange="handleNumberSelect()">
                          <label for="num${file.size}">${file.name}</label>
                        </div>
                    `).join('');
        document.getElementById('StoreWifi').innerHTML = list;
      });
  }

  // 获取附近有什么WIFI
  function GetWifiInfo() {
    fetch('/GetWifiInfo')
      .then(response => response.json())
      .then(files => {
        const list = files.map(file => `
                        <div class="wifi-item" onclick="showPasswordInput(this)">${file.name}</div>
                    `).join('');
        document.getElementById('liebiao').innerHTML = list;
      });
  }

  // 进入密码填写界面
  function showPasswordInput(element) {
    document.getElementById("liebiao").classList.add("hidden");
    document.getElementById("passwordForm").classList.remove("hidden");
    document.getElementById("ssid").value = element.textContent;
  }

  // 返回WIFI扫描界面
  function ReturnInterface() {
    document.getElementById("liebiao").classList.remove("hidden");
    document.getElementById("passwordForm").classList.add("hidden");
    // 重置表单状态
    document.getElementById("numberSelect").classList.remove("show");
    document.querySelectorAll('input[name="number"]').forEach(radio => radio.checked = false);
  }

  // 显示选择被覆盖WIFI信息
  function handleConnect() {
    const ssid = document.getElementById("ssid").value;
    const password = document.getElementById("password").value;

    if (!ssid || !password) {
      alert("请先填写WiFi名称和密码");
      return;
    }

    // 显示数字选择区域
    document.getElementById("numberSelect").classList.add("show");

    // 隐藏不需要的内容
    document.getElementById("ssid").classList.add("hidden");
    document.getElementById("password").classList.add("hidden");
    document.getElementById("ButtonJoin").classList.add("hidden");
    document.getElementById("ButtonReturn").classList.add("hidden");
    document.getElementById("WifiName").classList.add("hidden");
    document.getElementById("WifiPassword").classList.add("hidden");
  }

  // 发送表单
  function handleNumberSelect() {
    const selected = document.querySelector('input[name="number"]:checked');
    if (selected) {
      // 自动提交表单
      document.getElementById("passwordForm").submit();
    }
  }

  GetStoreWifi();
  GetWifiInfo();
</script>

</html>
)rawliteral";
