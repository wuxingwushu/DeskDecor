#ifndef __Network_H
#define __Network_H

#include <string.h>
#include <ESPAsyncWebServer.h> // 主库

// const char *ssid = "KT-2.4G", *password = "aurex123456";
// const char* ssid = "CMCC-U55E", *password = "eku7dx5f";
// const char* ssid = "道生", *password = "369784512";
// const char* ssid = "USER_128978", *password = "70438480";
// const char* ssid = "888", *password = "Huangze123";

// 网络情况枚举
typedef enum
{
    Network_Not = 1, // 没网络
    Network_Ok = 2,  // 有网络
    Network_Wed = 3,  // 启动Wed服务
    LowScanMode = 4  // 低频扫描模式
} NetworkCase;

extern AsyncWebServer server;

// 连接wifi
NetworkCase ConnectWIFI();

// 启动WebServer服务
String WebServerFun();


// 查询WIFI
void GetWifiInfo(AsyncWebServerRequest *request);
// 获取以储存WIFI
void GetStoreWifi(AsyncWebServerRequest *request);
// 处理WiFi配置提交
void handleWifiConfig(AsyncWebServerRequest *request);

// 獲取設置
void GetSetInfo(AsyncWebServerRequest *request);
// 处理WiFi配置提交
void handleSetConfig(AsyncWebServerRequest *request);

// 重启
void handleRestart(AsyncWebServerRequest *request);


// 获取文件列表
void GetFileList(AsyncWebServerRequest *request);
// 文件下载
void FileDownload(AsyncWebServerRequest *request);
// 文件删除
void FileDeletion(AsyncWebServerRequest *request);
// 获取存储信息
void RetrieveStorageInformation(AsyncWebServerRequest *request);
// 文件上传处理
void FileUploadProcessing(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
// 格式化
void FormatFloppyDisk(AsyncWebServerRequest *request);

extern const char *RootHtml; // 主页
extern const char *FileHtml; // 文件
extern const char *SetHtml; // 设置
extern const char *WifiHtml; // Wifi

#endif
