#include "Hitokoto.h"
#include "DEV_Config.h"

DynamicJsonDocument doc(2000);

HitokotoInfo GetHitokoto(unsigned int AttemptCount){
  Debug("一言:\n");
  // 执行HTTP请求
  HTTPClient http;
  http.begin("https://v1.hitokoto.cn");//https://v1.hitokoto.cn/?lang=zh
  int httpCode;
  // 多次尝试获取内容
  while(AttemptCount--){
    Debug('.');
    DEV_Delay_ms(100);
    httpCode = http.GET();
    if(httpCode == HTTP_CODE_OK){
      break;
    }
  }
  Debug('\n');
  HitokotoInfo HInfo;
  // 判断内容是否获取成功
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Debug(payload + "\n");

    // 解析JSON
    deserializeJson(doc, payload);

    
    HInfo.hitokoto = doc["hitokoto"];
    HInfo.from = doc["from"];
    HInfo.Success = true;

    Debug("Hitokoto: ");
    Debug(HInfo.hitokoto );
    Debug("\nFrom: ");
    Debug(HInfo.from);
    Debug("\n");
  } else {
    HInfo.Success = false;
    Debug("HTTP request failed\n");
  }

  http.end();

  return HInfo;
}