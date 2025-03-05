#ifndef __PresentTime_H
#define __PresentTime_H

#include <NTPClient.h>
#include <WiFiUdp.h>
#include "EepromString.h"
#include "GUI_Paint.h"

// 储存当前时间相关信息
extern unsigned char TimeH, TimeM, TimeS;

// 当前时间信息
typedef struct PresentTimeInfo {
  bool Success;              // 是否进入非工作时间范围
  unsigned int PresentTime;  // 休眠时间
  String PresentStr;         // 显然内容
};

// 有网时获取当前时间信息
bool RequestPresentTime();

// 根据当前时间返回对应事件信息
PresentTimeInfo GetDelayTime();
PresentTimeInfo GetDelayTime(unsigned char H, unsigned char M);

#endif
