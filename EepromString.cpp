#include "EepromString.h"
#include "Debug.h"

// 将字符串写入EEPROM
void writeStringToEEPROM(int startAddress, String message)
{
  int length = message.length();

  // 将字符串的每个字符写入EEPROM
  for (int i = 0; i < length; i++)
  {
    EEPROM.write(startAddress + i, message[i]);
  }

  // 在字符串末尾添加一个NULL终止符
  EEPROM.write(startAddress + length, '\0');

  EEPROM.commit(); // 提交更改并保存到EEPROM
  Debug("数据已写入EEPROM！");
}

// 从EEPROM读取字符串
String readStringFromEEPROM(int startAddress)
{
  String message = "";
  char ch = EEPROM.read(startAddress);

  // 逐个读取字符直到遇到NULL终止符
  while (ch != '\0')
  {
    message += ch;
    startAddress++;
    ch = EEPROM.read(startAddress);
  }

  return message;
}