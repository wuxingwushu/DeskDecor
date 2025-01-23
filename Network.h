#ifndef __Network_H
#define __Network_H

#include <string.h>
#include <WebServer.h>

// const char *ssid = "KT-2.4G", *password = "aurex123456";
// const char* ssid = "CMCC-U55E", *password = "eku7dx5f";
// const char* ssid = "道生", *password = "369784512";
// const char* ssid = "USER_128978", *password = "70438480";

// 网络情况枚举
typedef enum
{
    Network_Not = 1, // 没网络
    Network_Ok = 2,  // 有网络
    Network_Wed = 3  // 启动Wed服务
} NetworkCase;

extern WebServer server;

// 连接wifi
NetworkCase ConnectWIFI();

// 启动WebServer服务
String WebServerFun();

// 根路径请求的处理函数
void handleRoot();

// 設置WIFI網頁
void handleWifi();
// 处理WiFi配置提交
void handleWifiConfig();
// 設置界面
void handleSet();
// 处理WiFi配置提交
void handleSetConfig();
// 重启
void handleRestart();

extern const char *RootHtml; // 主页

#endif
