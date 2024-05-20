# epd2in13_V4 墨水屏一言

## 简介

使用ESP32控制墨水屏，每隔两分钟调用一言API获取每日一句，并显示在屏幕上。充满电可以运行6天时间

## 硬件
[epd2in13_V4](https://www.waveshare.net/wiki/2.13inch_e-Paper_Cloud_Module#.E5.8E.9F.E7.90.86.E5.9B.BE)
- 主控芯片：ESP32
- 墨水屏：2.13寸e-Paper 2in13 V4
- 电源：3.7V 1800mAh电池

## 依赖

字体文件生成使用的是[TestRoutineSet - TTF_bin](https://github.com/wuxingwushu/TestRoutineSet/tree/main/TTF_bin)的Freetype测试程序

图片ImageData.h文件生成使用的是[TestRoutineSet - Dithering](https://github.com/wuxingwushu/TestRoutineSet/tree/main/Dithering)的Freetype测试程序

# 效果
## 每隔2分钟更新显示内容
![示例图片1](./image/1.jpg)
## 没有WIFI时显示储存的图片
![示例图片2](./image/2.jpg)

