#include "Hitokoto.h"
#include "DEV_Config.h"

DynamicJsonDocument doc(2000);

HitokotoInfo GetHitokoto(){
  // 执行HTTP请求
  HTTPClient http;
  http.begin("https://v1.hitokoto.cn");//https://v1.hitokoto.cn/?lang=zh
  DEV_Delay_ms(100);
  int httpCode = http.GET();
  HitokotoInfo HInfo;
  HInfo.Success = false;
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Debug("\n");
    Debug(payload);

    // 解析JSON
    deserializeJson(doc, payload);

    
    HInfo.hitokoto = doc["hitokoto"];
    HInfo.from = doc["from"];
    HInfo.Success = true;

    Debug("\nHitokoto: ");
    Debug(HInfo.hitokoto);
    Debug("\nFrom: ");
    Debug(HInfo.from);
  } else {
    HInfo.Success = false;
    Debug("HTTP request failed");
  }

  http.end();

  return HInfo;
}