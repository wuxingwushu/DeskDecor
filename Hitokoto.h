#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Debug.h"

// 一言信息
typedef struct HitokotoInfo
{
  bool Success;// 是否成功
  const char* hitokoto;// 一言句子
  const char* from;// 作者
};

// 获取一言
HitokotoInfo GetHitokoto();