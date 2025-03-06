#include "Sentence.h"
#include "DEV_Config.h"
#include "EepromString.h"

DynamicJsonDocument doc(4000);

SentenceInfo GetSentence(){
  unsigned char Passage;
  SentenceInfo info;
  EEPROM.get(SentenceAPIPassageAddr, Passage);
  if(Passage <= 1){
    Debug("有没有搞错啊! API都没有选耶!");
    info.Success = true;
    info.hitokoto = "有没有搞错啊! API都没有选耶!";
    info.from = "开发者";
    return info;
  }
  if(Passage & 0x01){// 循环模式
    unsigned char API;
    EEPROM.get(SentenceAPIAddr, API);
    // 让他在合理范围内
    if((API == 0) || (API >= 4)){
      API = 1;
    }
    while(true){
      if(Passage & (1 << API)){
        break;
      }
      ++API;
      if(API >= 4){
        API = 1;
      }
    }
    ++API;
    EEPROM.put(SentenceAPIAddr, API);  // 存储
    EEPROM.commit();
    Passage = 1 << (API - 1);
  }
  info = GetSentence(SentenceAPI(Passage), 10);
  info.StrSize = String(info.hitokoto).length();
  return info;

}

SentenceInfo GetSentence(SentenceAPI API, unsigned int AttemptCount){
    switch(API){
      case HitokotoAPI: return GetHitokoto(AttemptCount);
      case ONEAPI: return GetONE(AttemptCount);
      case GreenTangerineAPI: return GetGreenTangerine(AttemptCount);
      default: Debug("不存在 SentenceAPI Enum");
    }
    SentenceInfo info;
    info.Success = true;
    info.hitokoto = "有没有搞错啊! 你又写出BUG了!";
    info.from = "开发者";
    return info;
}

SentenceInfo GetHitokoto(unsigned int AttemptCount){
  Debug("一言:\n");
  // 执行HTTP请求
  HTTPClient http;
  http.begin("https://v1.hitokoto.cn"); // https://v1.hitokoto.cn/?lang=zh
  int httpCode;
  // 多次尝试获取内容
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
  SentenceInfo HInfo;
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
    HInfo.Success = true;// 虽然获取内容失败了，但还是显示固定内容
    HInfo.hitokoto = "\"一言\"获取内容失败！";
    HInfo.from = "开发者";
    Debug("HTTP Hitokoto failed\n");
  }
  http.end();
  return HInfo;
}


SentenceInfo GetONE(unsigned int AttemptCount){
  Debug("ONE:\n");
  // 执行HTTP请求
  HTTPClient http;
  http.begin("https://api.xygeng.cn/one");
  int httpCode;
  // 多次尝试获取内容
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
  SentenceInfo OInfo;
  // 判断内容是否获取成功
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Debug(payload + "\n");

    // 解析JSON
    deserializeJson(doc, payload);
    
    OInfo.hitokoto = doc["data"]["content"];
    OInfo.from = doc["data"]["origin"];
    OInfo.Success = true;

    Debug("Hitokoto: ");
    Debug(OInfo.hitokoto );
    Debug("\nFrom: ");
    Debug(OInfo.from);
    Debug("\n");
  } else {
    OInfo.Success = true;// 虽然获取内容失败了，但还是显示固定内容
    OInfo.hitokoto = "\"ONE\"获取内容失败！";
    OInfo.from = "开发者";
    Debug("HTTP ONE failed\n");
  }
  http.end();
  return OInfo;
}

SentenceInfo GetGreenTangerine(unsigned int AttemptCount){
  Debug("青桔:\n");
  // 执行HTTP请求
  HTTPClient http;
  http.begin("https://api.qjqq.cn/api/Yi");
  int httpCode;
  // 多次尝试获取内容
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
  SentenceInfo QInfo;
  // 判断内容是否获取成功
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Debug(payload + "\n");

    // 解析JSON
    deserializeJson(doc, payload);

    QInfo.hitokoto = doc["hitokoto"];
    QInfo.from = doc["from"];
    QInfo.Success = true;

    Debug("Hitokoto: ");
    Debug(QInfo.hitokoto );
    Debug("\nFrom: ");
    Debug(QInfo.from);
    Debug("\n");
  } else {
    QInfo.Success = true;// 虽然获取内容失败了，但还是显示固定内容
    QInfo.hitokoto = "\"青桔\"获取内容失败！";
    QInfo.from = "开发者";
    Debug("HTTP GetGreenTangerine failed\n");
  }
  http.end();
  return QInfo;
}
