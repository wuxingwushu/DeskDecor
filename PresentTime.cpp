#include "PresentTime.h"

WiFiUDP ntpUDP;
NTPClient TimeNTPClient(ntpUDP, "ntp.aliyun.com", 60 * 60 * 8, 60 * 1000);

void RequestPresentTime() {
  TimeNTPClient.begin();
  TimeNTPClient.update();
}

PresentTimeInfo GetDelayTime() {
  PresentTimeInfo DelayTime;
  DelayTime.Success = false;
  DelayTime.PresentStr = "";
  // 獲取設置的工作時間範圍
  unsigned char StartHours, StartMinutes, EndHours, EndMinutes;
  EEPROM.get(StartTimeHoursAddr, StartHours);      // 獲取開始 時
  EEPROM.get(StartTimeMinutesAddr, StartMinutes);  // 獲取開始 分
  EEPROM.get(EndTimeHoursAddr, EndHours);          // 獲取結束 時
  EEPROM.get(EndTimeMinutesAddr, EndMinutes);      // 獲取結束 分
  // 獲取當前時間
  int TimeH = TimeNTPClient.getHours();    // 获取时
  int TimeM = TimeNTPClient.getMinutes();  // 获取分
  // 判斷是否在工作時間範圍外
  if (TimeH >= EndHours) {
    if (!((TimeH == EndHours) && (TimeM < EndMinutes))) {
      DelayTime.PresentTime = (24 - TimeH + StartHours) * 60 - TimeM;  // 工作範圍外到工作時間開始的時間差（分）
      DelayTime.Success = true;
    }
  } else if (TimeH <= StartHours) {
    if ((TimeH == StartHours) && (TimeM < StartMinutes)) {
      DelayTime.PresentTime = (StartHours - TimeH) * 60 - TimeM;  // 工作範圍外到工作時間開始的時間差（分）
      DelayTime.Success = true;
    }
  }
  if (DelayTime.Success) {
    String DormantTime = "休眠到 ";
    if (StartHours < 10) {
      DormantTime += "0" + String(StartHours);
    } else {
      DormantTime += String(StartHours);
    }
    DormantTime += ":";
    if (StartMinutes < 10) {
      DormantTime += "0" + String(StartMinutes);
    } else {
      DormantTime += String(StartMinutes);
    }
    DelayTime.PresentStr = DormantTime;
  }
  return DelayTime;
}
