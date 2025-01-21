#include <string.h>
#include <EEPROM.h>


// 将字符串写入EEPROM
void writeStringToEEPROM(int startAddress, String message);
// 从EEPROM读取字符串
String readStringFromEEPROM(int startAddress);