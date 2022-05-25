#ifndef WEARABLE_DATA_T_H
#define WEARABLE_DATA_T_H

#include "pinDebug.h"

typedef struct{
  float temperature;            // 4 bytes - little endian
  float humidity;               // 4 bytes - little endian
  uint32_t  particle_2_5_count; // 4 bytes - little endian
  uint32_t voc_data;            // 4 bytes
  uint32_t co2_data;            // 4 bytes
} wearable_data_t;

static void printWearableData(wearable_data_t wd)
{
#ifdef PIN_SERIAL_ON
  Serial.print("Wearable Data: T:");
  Serial.print(wd.temperature);
  Serial.print(", H:");
  Serial.print(wd.humidity);
  Serial.print(", PM2.5:");
  Serial.print(wd.particle_2_5_count);
  Serial.print(", VOC:");
  Serial.print(wd.voc_data);
  Serial.print(", CO2:");
  Serial.println(wd.co2_data);
  Serial.flush();
  #endif
}
#endif
