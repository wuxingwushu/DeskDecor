#ifndef __Hitokoto_H
#define __Hitokoto_H

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

/**
 * @brief 获取一言
 * @param AttemptCount 尝试获取次数
 * @return 一言信息 */
HitokotoInfo GetHitokoto(unsigned int AttemptCount = 1);

#endif
