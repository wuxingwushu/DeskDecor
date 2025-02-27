#include "PresentTime.h"

WiFiUDP ntpUDP;
NTPClient TimeNTPClient(ntpUDP, "ntp.aliyun.com", 60 * 60 * 8, 60 * 1000);
unsigned char TimeH, TimeM, TimeS;

void RequestPresentTime() {
  TimeNTPClient.begin();
  TimeNTPClient.update();
}

PresentTimeInfo GetDelayTime() {
  PresentTimeInfo DelayTime;
  DelayTime.Success = false;
  DelayTime.PresentStr = "";
  // 获取设置的工作时间范围
  unsigned char StartHours, StartMinutes, EndHours, EndMinutes;
  EEPROM.get(StartTimeHoursAddr, StartHours);      // 获取开始 时
  EEPROM.get(StartTimeMinutesAddr, StartMinutes);  // 获取开始 分
  EEPROM.get(EndTimeHoursAddr, EndHours);          // 获取结束 时
  EEPROM.get(EndTimeMinutesAddr, EndMinutes);      // 获取结束 分

  // 获取当前时间
  TimeH = TimeNTPClient.getHours();    // 获取时
  TimeM = TimeNTPClient.getMinutes();  // 获取分
  TimeS = TimeNTPClient.getSeconds();   // 获取秒

  // 开始时间 和 结束时间 相同 代表不停止工作
  if((StartHours == EndHours) && (StartMinutes == EndMinutes)){
    return DelayTime;
  }

  // 转换为分钟数
  unsigned int StartTotal = StartHours * 60 + StartMinutes;// 开始时间（分为单位）
  unsigned int EndTotal = EndHours * 60 + EndMinutes;// 结束时间（分为单位）
  unsigned int TimeTotal = TimeH * 60 + TimeM;// 当前时间（分为单位）
  unsigned char DormantWork = 0;// 是否工作

  if(StartTotal < EndTotal){// 工作时间 非跨天情况
    if((TimeTotal < StartTotal) || (TimeTotal >= EndTotal)){
      DormantWork = 1;
    }
  }else{// 工作时间 跨天情况
    if((TimeTotal < StartTotal) && (TimeTotal >= EndTotal)){
      DormantWork = 2;
    }
  }

  if(DormantWork == 1){ // 休眠时间跨天情况
    if(TimeTotal > EndTotal){
      DelayTime.PresentTime = (24 * 60) - TimeTotal + StartTotal;
    }else{
      DelayTime.PresentTime = StartTotal - TimeTotal;
    }
  }else if(DormantWork == 2){// 休眠时间不跨天情况
    DelayTime.PresentTime = StartTotal - TimeTotal;
  }

  // 是否在休眠时间范围内
  if (DormantWork != 0) {
    DelayTime.Success = true;
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
