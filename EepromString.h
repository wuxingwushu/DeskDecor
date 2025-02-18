#ifndef __EepromString_H
#define __EepromString_H

#include <string.h>
#include <EEPROM.h>

#define EepromBufferSize 512  // 存储空间大小
#define ImgIndexAddr 0        // 下一张图片索引
#define SleepValueAddr 4      // 休眠时间

#define StartTimeHAddr 8  // 开始时间 时
#define StartTimeMAddr 9  // 开始时间 分
#define EndTimeHAddr 10   // 结束时间 时
#define EndTimeMAddr 11   // 结束时间 分

#define LatitudeAddr 12   // 纬度
#define LongitudeAddr 16  // 经度

#define WifiNameAddr 20   // WIFI 名字
#define WifiPassAddr 400  // WIFI 密码

// 将字符串写入EEPROM
void writeStringToEEPROM(int startAddress, String message);
// 从EEPROM读取字符串
String readStringFromEEPROM(int startAddress);

#endif
