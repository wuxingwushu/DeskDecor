#include <esp_adc_cal.h>
#include <driver/adc.h>

#define DEFAULT_VREF    1233            // 默认1.1V的参考电压
#define NO_OF_SAMPLES   64              // ADC采样次数
#define ADC_WIDTH   ADC_WIDTH_12Bit     // ADC 12位宽度
#define ADC_ATTEN   ADC_ATTEN_DB_11     // 11dB衰减器
#define ADC_PIN     ADC1_CHANNEL_0      // ADC通道0

#define MinVoltage 1230 // voltage 最小1230
#define MaxVoltage 1390 // voltage 最大1390
#define VoltageRange (MaxVoltage - MinVoltage) // 电压量程

// 初始化ADC
void ADC_Init();

// 读取电压百分比
float ReadADC();