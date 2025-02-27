#include "FileSystem.h"
#include "DEV_Config.h"
#include "Debug.h"

void ShowFileInfo() {
    // 新版API直接调用成员函数
    Debug("总空间:" + String(SPIFFS.totalBytes()) + " bytes\n");
    Debug("使用空间:" + String(SPIFFS.usedBytes()) + " bytes\n");
    Debug("剩余空间:" + String(SPIFFS.totalBytes() - SPIFFS.usedBytes()) + " bytes\n");
    //Debug("最大连续块:" + String(SPIFFS.blockSize() * SPIFFS.maxContiguousBlocks()) + " bytes\n");

    File root = SPIFFS.open("/");
    if (!root) {
        Debug("打开根目录失败！");
        return;
    }
    if (!root.isDirectory()) {
        Debug("不是文件夹！");
        return;
    }

    File fileS = root.openNextFile();
    while (fileS) {
        Debug("文件:"); Debug(fileS.name()); 
        Debug("\t大小:"); Debug(fileS.size()); Debug("\n");
        fileS.close();
        fileS = root.openNextFile();
    }
    root.close();
}
