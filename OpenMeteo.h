#ifndef __Openmeteo_H
#define __Openmeteo_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Debug.h"

// 天气信息
typedef struct OpenMeteoInfo {
  bool Success;          // 是否成功
  String Temperature;    // 温度
  unsigned int Weather;  // 天气
};

/**
 * @brief 获取天气
 * @return 天气信息 */
OpenMeteoInfo GetOpenMeteo();

/**
 * @brief 天气代码转天气描述
 * @param WeatherCode 天气代码 
 * @return 天气描述 */
String GetMeteoToString(unsigned int WeatherCode);

#endif
