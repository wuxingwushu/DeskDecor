#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Debug.h"

typedef struct HitokotoInfo
{
  bool Success;
  const char* hitokoto;
  const char* from;
};


HitokotoInfo GetHitokoto();