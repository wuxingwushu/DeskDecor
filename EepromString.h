#ifndef __EepromString_H
#define __EepromString_H

#include <string.h>
#include <EEPROM.h>

#define EepromBufferSize 512 // 存储空间大小
#define ImgIndexAddr 0       // 下一张图片索引
#define SleepValueAddr 4     // 休眠时间
#define WifiNameAddr 8       // WIFI 名字
#define WifiPassAddr 400     // WIFI 密码

// 将字符串写入EEPROM
void writeStringToEEPROM(int startAddress, String message);
// 从EEPROM读取字符串
String readStringFromEEPROM(int startAddress);

#endif
