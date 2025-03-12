#include "OpenMeteo.h"
#include "DEV_Config.h"
#include "EepromString.h"

OpenMeteoInfo GetOpenMeteo(unsigned int AttemptCount) {
  Debug("天气:\n");
  String OpenMeteoHtml1 = "https://api.open-meteo.com/v1/forecast?latitude=";
  String OpenMeteoHtml2 = "&longitude=";
  String OpenMeteoHtml3 = "&current=temperature_2m,weather_code&timezone=Asia%2FSingapore&forecast_days=1";
  float Lxxitude;
  EEPROM.get(LatitudeAddr, Lxxitude);
  OpenMeteoHtml1 += String(Lxxitude) + OpenMeteoHtml2;
  EEPROM.get(LongitudeAddr, Lxxitude);
  OpenMeteoHtml1 += String(Lxxitude) + OpenMeteoHtml3;
  Debug(OpenMeteoHtml1 + "\n");

  // 执行HTTP请求
  HTTPClient http;
  http.begin(OpenMeteoHtml1);
  int httpCode;
  while(AttemptCount--){
    Debug('.');
    httpCode = http.GET();
    if(httpCode == HTTP_CODE_OK){
      break;
    }else{
      DEV_Delay_ms(100);
    }
  }
  Debug('\n');
  OpenMeteoInfo OInfo;
  // 判断是否获取内容失败
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Debug(payload + "\n");

    // 解析JSON
    DynamicJsonDocument doc(2000);
    deserializeJson(doc, payload);

    // 当前天气
    String weather = doc["current"]["weather_code"];  // 当天天气代码
    OInfo.Weather = weather.toInt();
    String temperature = doc["current"]["temperature_2m"];  //当天温度
    OInfo.Temperature = temperature;
    OInfo.Success = true;
    Debug("天氣代碼: " + weather + "\t溫度: " + temperature + "\n");
  }else{
    OInfo.Success = false;
    Debug("Error: GetOpenMeteo() fail !\n");
  }
  http.end();

  return OInfo;
}


/*
0 晴天
1~3 多云
45~48 雾
51~55 毛毛雨
61~65 雨
71~75 雪
80~82 阵雨
95~99 雷雨
*/
String GetMeteoToString(unsigned int WeatherCode) {
  // "☃☁☂☀ ☀☁☂☃☔☼⚡⛆⛅⛇⛈"
  switch (WeatherCode) {
    case 0: return "晴天";
    case 1: return "晴朗";
    case 2: return "部分多云";
    case 3: return "多云";
    case 45: return "雾";
    case 48: return "沉积雾淞";
    case 51: return "小毛毛雨";
    case 53: return "中毛毛雨";
    case 55: return "大毛毛雨";
    case 56: return "小冻毛毛雨";
    case 57: return "大冻毛毛雨";
    case 61: return "小雨";
    case 63: return "中雨";
    case 65: return "大雨";
    case 66: return "小冻雨";
    case 67: return "大冻雨";
    case 71: return "小雪";
    case 73: return "中雪";
    case 75: return "大雪";
    case 77: return "雪粒";
    case 81: return "小阵雨";
    case 82: return "中阵雨";
    case 83: return "大阵雨";
    case 85: return "小阵雪";
    case 86: return "大阵雪";
    case 95: return "雷暴";
    case 96: return "雷暴小冰雹";
    case 99: return "雷暴大冰雹";
    default: return String(WeatherCode);
  }
}
