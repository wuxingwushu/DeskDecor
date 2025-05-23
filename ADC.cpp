#include "ADC.h"
#include "Debug.h"
#include <stdlib.h>
#include <Arduino.h>

esp_adc_cal_characteristics_t *adc_chars;

void ADC_Init(){
  adc1_config_width(ADC_WIDTH);  // 设置ADC为12位宽度
  adc1_config_channel_atten(ADC_PIN, ADC_ATTEN);  // 配置ADC通道为6dB衰减器

  // 使用eFuse校准ADC，并获取校准值
  adc_chars = (esp_adc_cal_characteristics_t *) malloc(sizeof(esp_adc_cal_characteristics_t));
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN, ADC_WIDTH, DEFAULT_VREF, adc_chars);
  if (val_type == ESP_ADC_CAL_VAL_NOT_SUPPORTED) {
    Debug("校准失败，请检查硬件!\n");
    return;
  }
}

float ReadADC(){
  uint32_t adc_reading = 0;
  for (int i = 0; i < NO_OF_SAMPLES; i++) {
    adc_reading += adc1_get_raw((adc1_channel_t) ADC_PIN);
    delay(10);
  }

  adc_reading /= NO_OF_SAMPLES;
  int voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);

  Debug("ADC Reading: " + String(adc_reading) + "\tVoltage: " + String(voltage) + "\t" + String(float(voltage - MinVoltage) / VoltageRange) + "\n");

  return float(voltage - MinVoltage) / VoltageRange;
}
