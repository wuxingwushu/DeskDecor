#ifndef __Openmeteo_H
#define __Openmeteo_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Debug.h"





/*
0 晴天
1~3 多云
61~65 雨
71~75 雪
95~99 雷雨
45~48 雾
*/

// const char *OpenMeteoHtml = "https://api.open-meteo.com/v1/forecast?latitude=22.9882&longitude=114.3198&hourly=temperature_2m,weather_code&daily=weather_code&timezone=Asia%2FSingapore&forecast_days=1";


// 天气信息
typedef struct OpenMeteoInfo
{
    bool Success;         // 是否成功
    String Temperature;   // 温度
    unsigned int Weather; // 天气
};

OpenMeteoInfo GetOpenMeteo();

String GetMeteoToString(unsigned int WeatherCode);



#endif
