#include "PresentTime.h"

WiFiUDP ntpUDP;
NTPClient TimeNTPClient(ntpUDP, "ntp.aliyun.com", 60 * 60 * 8, 60 * 1000);
unsigned char TimeD, TimeH, TimeM, TimeS;


String GetTimeDayStr(unsigned char day) {
  switch (day) {
    case 0x01: return "星期日";
    case 0x02: return "星期一";
    case 0x04: return "星期二";
    case 0x08: return "星期三";
    case 0x10: return "星期四";
    case 0x20: return "星期五";
    case 0x40: return "星期六";
    default: Debug("星期几出错啦\n");
  }
  return String((int)day);
}

bool RequestPresentTime() {
  char TimeZone;
  EEPROM.get(TimeZoneAddr, TimeZone);
  Debug("时区:" + String(int(TimeZone)) + "\n");
  TimeNTPClient.setTimeOffset(60 * 60 * int(TimeZone));
  TimeNTPClient.begin();
  unsigned int Count = 3;
  while (Count--) {
    if (TimeNTPClient.update()) {
      return true;
    }
    DEV_Delay_ms(100);
  }
  return false;
}

PresentTimeInfo GetDelayTime() {
  // 获取当前时间
  TimeD = TimeNTPClient.getDay();      // 获取星期几
  TimeH = TimeNTPClient.getHours();    // 获取时
  TimeM = TimeNTPClient.getMinutes();  // 获取分
  TimeS = TimeNTPClient.getSeconds();  // 获取秒

  Debug(String((int)TimeD) + ":" + String((int)TimeH) + ":" + String((int)TimeM) + ":" + String((int)TimeS) + "\n");
  return GetDelayTime(TimeD, TimeH, TimeM);
}


PresentTimeInfo GetDelayTime(unsigned char D, unsigned char H, unsigned char M) {
  PresentTimeInfo DelayTime;
  DelayTime.Success = false;
  DelayTime.PresentStr = "";
  DelayTime.PresentTime = 0;
  // 获取设置的工作时间范围
  unsigned char WorkDay, StartHours, StartMinutes, EndHours, EndMinutes;
  EEPROM.get(WorkDayAddr, WorkDay);  // 获取开始 时
  Debug("WorkDayAddr: " + String((int)WorkDay) + "\n");
  EEPROM.get(StartTimeHoursAddr, StartHours);      // 获取开始 时
  EEPROM.get(StartTimeMinutesAddr, StartMinutes);  // 获取开始 分
  EEPROM.get(EndTimeHoursAddr, EndHours);          // 获取结束 时
  EEPROM.get(EndTimeMinutesAddr, EndMinutes);      // 获取结束 分

  // 开始时间 和 结束时间 相同 代表不停止工作
  if ((StartHours == EndHours) && (StartMinutes == EndMinutes)) {
    return DelayTime;
  }

  // 转换为分钟数
  unsigned int StartTotal = StartHours * 60 + StartMinutes;  // 开始时间（分为单位）
  unsigned int EndTotal = EndHours * 60 + EndMinutes;        // 结束时间（分为单位）
  unsigned int TimeTotal = H * 60 + M;                       // 当前时间（分为单位）
  unsigned char DormantWork = 0;                             // 是否工作

  if (StartTotal < EndTotal) {  // 工作时间 非跨天情况
    if ((TimeTotal < StartTotal) || (TimeTotal >= EndTotal)) {
      DormantWork = 1;
    }
  } else {  // 工作时间 跨天情况
    if ((TimeTotal < StartTotal) && (TimeTotal >= EndTotal)) {
      DormantWork = 2;
    }
  }
  
  // 判断当前是否工作日 (WorkDay按bit0~bit6对应周日~周六)
  bool isWorkDay = (WorkDay & (0x01 << D)) != 0;
  if (isWorkDay == 0) {
    Debug("非工作日\n");
    DormantWork = 0;
    DelayTime.PresentTime += 24 * 60 - TimeTotal + StartTotal;
  }

  if (DormantWork == 1) {  // 休眠时间跨天情况
    if (TimeTotal >= EndTotal) {
      DelayTime.PresentTime = (24 * 60) - TimeTotal + StartTotal;
    } else {
      DelayTime.PresentTime = StartTotal - TimeTotal;
    }
  } else if (DormantWork == 2) {  // 休眠时间不跨天情况
    DelayTime.PresentTime = StartTotal - TimeTotal;
  }  

  // 是否在休眠时间范围内
  if ((isWorkDay == 0) | DormantWork) {
    DelayTime.Success = true;

    if (isWorkDay && (DormantWork == 1) && (TimeTotal < StartTotal)) {
      return DelayTime;
    }

    int daysChecked = -1;
    do {
      ++daysChecked;
      ++D;
      D %= 7;
    } while (((WorkDay & (0x01 << D)) == 0) && (daysChecked < 7));
    if (daysChecked >= 7) { daysChecked = 0; }
    DelayTime.PresentTime += 24 * 60 * daysChecked;
    --D;
    /*
    String DormantTime = "休眠到" + GetTimeDayStr(0x01 << D) + "的 ";
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
    */
  }
  return DelayTime;
}
