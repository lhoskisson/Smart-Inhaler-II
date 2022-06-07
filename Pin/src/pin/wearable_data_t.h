#ifndef WEARABLE_DATA_T_H
#define WEARABLE_DATA_T_H

#include "pinDebug.h"

/*
 * This data type is what will be written and indicated to the app
 * Note that for indication the payload has to be less than or equal to 20 bytes
 * https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v15.0.0%2Flib_ble_gatt.html 
 */
typedef struct{
  float temperature;            // 4 bytes
  float humidity;               // 4 bytes
  uint16_t  particle_2_5_count; // 2 bytes
  uint16_t  particle_10_count;  // 2 bytes
  uint16_t voc_data;            // 2 bytes
  uint16_t co2_data;            // 2 bytes
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
  Serial.print(", PM10:");
  Serial.print(wd.particle_10_count);
  Serial.print(", VOC:");
  Serial.print(wd.voc_data);
  Serial.print(", CO2:");
  Serial.println(wd.co2_data);
  Serial.flush();
  #endif
}
#endif
