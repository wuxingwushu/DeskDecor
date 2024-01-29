#include <esp_adc_cal.h>
#include <driver/adc.h>


#define DEFAULT_VREF    1100    // 默认1.1V的参考电压
#define NO_OF_SAMPLES   64      // ADC采样次数
#define ADC_WIDTH       ADC_WIDTH_12Bit  // ADC 12位宽度
#define ADC_ATTEN       ADC_ATTEN_DB_11    // 11dB衰减器
#define ADC_PIN         ADC1_CHANNEL_4




void ADC_Init();

float ReadADC();