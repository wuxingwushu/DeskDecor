#ifndef __Hitokoto_H
#define __Hitokoto_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Debug.h"

// https://api.oddfar.com/yl/q.php 官网 https://oddfar.com/archives/49/ 返回值 { "code": "200", "msg": "success", "type": "2004", "text": "我点燃了火，却控制不了它。" }
// https://uapis.cn/api/say 官网 https://uapis.cn/docs/say

// 句子信息
typedef struct SentenceInfo
{
  bool Success;// 是否成功
  const char* hitokoto;// 一言句子
  const char* from;// 作者
};

typedef enum
{
    HitokotoAPI = 1 << 1,// 一言
    ONEAPI = 1 << 2, // ONE
    GreenTangerineAPI = 1 << 3  // 青桔
} SentenceAPI;

/**
 * @brief 获取 每日一句
 * @return 句子信息 */
SentenceInfo GetSentence();

/**
 * @brief 获取 每日一句
 * @param API 选择API
 * @param AttemptCount 尝试获取次数
 * @return 句子信息 */
SentenceInfo GetSentence(SentenceAPI API, unsigned int AttemptCount = 1);

/**
 * @brief 获取 一言 每日一句
 * @param AttemptCount 尝试获取次数
 * @return 句子信息 */
SentenceInfo GetHitokoto(unsigned int AttemptCount = 1);

/**
 * @brief 获取 ONE 每日一句
 * @param AttemptCount 尝试获取次数
 * @return 句子信息 */
SentenceInfo GetONE(unsigned int AttemptCount = 1);

/**
 * @brief 获取 青桔 每日一句
 * @param AttemptCount 尝试获取次数
 * @return 句子信息 */
SentenceInfo GetGreenTangerine(unsigned int AttemptCount = 1);

#endif
