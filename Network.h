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
    Network_Wed = 3 // 启动Wed服务
} NetworkCase;

// 试图连接网络
NetworkCase ConnectWIFI();

// 开启Wed服务
void WedServerFun();

void NetworkhandleClient();

// 根路径请求的处理函数
void handleRoot();
// 处理WiFi配置提交
void handleConfig();
