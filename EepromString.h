#ifndef __EepromString_H
#define __EepromString_H

#include <string.h>
#include <EEPROM.h>

#define EepromBufferSize 1024  // 存储空间大小
#define ImgIndexAddr 0        // 下一张图片索引（int）
#define SleepValueAddr 4      // 休眠时间（int）

#define StartTimeHoursAddr 8  // 开始时间 时（unsigned char）
#define StartTimeMinutesAddr 9  // 开始时间 分（unsigned char）
#define EndTimeHoursAddr 10   // 结束时间 时（unsigned char）
#define EndTimeMinutesAddr 11   // 结束时间 分（unsigned char）

#define LatitudeAddr 12   // 纬度（float）
#define LongitudeAddr 16  // 经度（float）

#define SentenceAPIAddr 20  // 当前是调用哪个 API（unsigned char）
#define SentenceAPIPassageAddr 21  // 开启的API有那些（unsigned char）（第一个bit代表是否开始循环，最多7个API类型）

#define WiFiStrInterval 200  // WIFI 数据间隔（WIFI 数据空间大小）
#define WifiNameAddr 24   // WIFI 名字（char*）
#define WifiPassAddr (WifiNameAddr + (WiFiStrInterval / 2))  // WIFI 密码（char*）
#define WifiDateMaxSize ((EepromBufferSize - WifiNameAddr) / WiFiStrInterval) // 可以储存 WIFI数据 数量

// 将字符串写入EEPROM
void writeStringToEEPROM(int startAddress, String message);
// 从EEPROM读取字符串
String readStringFromEEPROM(int startAddress);

#endif
