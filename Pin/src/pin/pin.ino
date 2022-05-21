#include <bluefruit.h>
#include "wearable_data_t.h"
#include "wearableDataQueue.h"
#include <SPI.h>
#include <Wire.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_PM25AQI.h>
#include <Adafruit_SGP30.h>
#include <Adafruit_PM25AQI.h>
#include <Adafruit_Sensor.h>

#include "pinDebug.h"

wearableDataQueue q;

Adafruit_PM25AQI aqi = Adafruit_PM25AQI();
PM25_AQI_Data PMdata; //PM2.5 sensor


Adafruit_SGP30 sgp; //VOC SGP30 sensor

//Temperature sensor
#define DHTPIN 5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//Pin UUIDs
#define PIN_SERVICE_UUID                  "25380284e1b6489abbcf97d8f7470aa4"
#define PIN_DATA_CHARACTERISTIC_UUID      "c3856cfa4af64d0da9a05ed875d937cc"
#define PIN_APPEARANCE 0x0552   //BLE Apperance code for multi-sensor

//setup Pin UUID classes
BLEUuid pinServiceUuid(PIN_SERVICE_UUID);
BLEUuid pinDataCharacteristicUuid(PIN_DATA_CHARACTERISTIC_UUID);

//create service and characteristic classes
BLEService pinService(pinServiceUuid);
BLECharacteristic pinDataCharacteristic(pinDataCharacteristicUuid);

BLEDis bledis;

void setup() 
{
#ifdef PIN_SERIAL_ON
  //setup serial connection for debug
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Serial Connected");
  Serial.flush();
#endif

  /*
   * QUEUE SETUP
   */
   q.begin();

  /*
   * BLEDIS setup
   */
   bledis.setModel("Bluefruit Feather52");
  
  /*
   * PIN BLE SETUP
   */
  pinDataCharacteristic.setProperties(CHR_PROPS_READ);
  pinDataCharacteristic.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  pinDataCharacteristic.setFixedLen(sizeof(wearable_data_t));
  
  //initialize ble settings
  Bluefruit.setName("Smart Inhaler");
  Bluefruit.begin();

  bledis.begin();
  
  //start pin service, and attach charactersitic
  pinService.begin();
  pinDataCharacteristic.begin();

  
  /*
   * BLE Advertising Setup
   */
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addUuid(pinServiceUuid); //android app looks for service uuid in advertising
  Bluefruit.Advertising.addService(pinService);
  Bluefruit.Advertising.addAppearance(PIN_APPEARANCE);
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.start();
  
  /*
   * NON BLE SETUP
   */
}

void loop() 
{
}

wearable_data_t getWearableData
{
  wearable_data_t wd;
  wd.voc_data = sgp.TVOC();
  wd.co2_data = sgp.CO2();
  wd.temp = dht.readTemperature();
  wd.humidity = dht.readHumidity();
  wd.PM25 = getPM25();
  wd.PM100 = getPM100();
  return wd;
}
