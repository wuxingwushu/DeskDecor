#include "OpenMeteo.h"
#include "DEV_Config.h"

const char *OpenMeteoHtml = "https://api.open-meteo.com/v1/forecast?latitude=22.9882&longitude=114.3198&current=temperature_2m,weather_code&timezone=Asia%2FSingapore&forecast_days=1";

unsigned char ShuPixData[10][2] = {
    {0xF6, 0xCE}, // 0
    {0x48, 0x24}, // 1
    {0xE7, 0xCE}, // 2
    {0xE7, 0x9E}, // 3
    {0xB7, 0x92}, // 4
    {0xF3, 0x9E}, // 5
    {0xF3, 0xDE}, // 6
    {0xE4, 0x92}, // 7
    {0xF7, 0xDE}, // 8
    {0xF7, 0x9E}  // 9
};

OpenMeteoInfo GetOpenMeteo(){
    // 执行HTTP请求
    HTTPClient http;
    http.begin(OpenMeteoHtml);
    DEV_Delay_ms(100);
    int httpCode = http.GET();
    OpenMeteoInfo OInfo;
    OInfo.Success = false;
    if(httpCode == HTTP_CODE_OK){
        String payload = http.getString();
        Debug(payload);

        // 解析JSON
        DynamicJsonDocument doc(4000);
        deserializeJson(doc, payload);

        // 当前天气
        String weather = doc["current"]["weather_code"];// 当天天气代码
        Debug(weather);
        OInfo.Weather = weather.toInt();
        String temperature = doc["current"]["temperature_2m"];//当天温度
        Debug(temperature);
        OInfo.Temperature = temperature;
        OInfo.Success = true;
    }
    return OInfo;
}


String GradeString(unsigned int lv){
    if(lv == 0){
        return "Ⅰ";
    }else if(lv == 1){
        return "Ⅱ";
    }else if(lv == 2){
        return "Ⅲ";
    }else if(lv == 3){
        return "Ⅳ";
    }else if(lv == 4){
        return "Ⅴ";
    }else if(lv == 5){
        return "Ⅵ";
    }else if(lv == 6){
        return "Ⅶ";
    }else{
        return String(lv);
    }
}

/*
0 晴天
1~3 多云
61~65 雨
71~75 雪
95~99 雷雨
45~48 雾
*/
String GetMeteoToString(unsigned int WeatherCode){
    if (0 == WeatherCode){
        return "晴天";
    }else if (3 >= WeatherCode){
        return "多云" + GradeString(WeatherCode - 1);
    }else if (48 >= WeatherCode){
        return "雾" + GradeString(WeatherCode - 45);
    }else if (65 >= WeatherCode){
        return "雨" + GradeString(WeatherCode - 61);
    }else if (75 >= WeatherCode){
        return "雪" + GradeString(WeatherCode - 71);
    }else if (99 >= WeatherCode){
        return "雷雨" + GradeString(WeatherCode - 95);
    }else{
        return String(WeatherCode);
    }
}


