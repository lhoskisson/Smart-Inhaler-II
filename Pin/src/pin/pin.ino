#include <bluefruit.h>
#include "wearable_data_t.h"
#include "wearableDataQueue.h"

#include "pinDebug.h"

wearableDataQueue q;

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

//create test data
wearable_data_t testData;

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
  //creating test data
  testData.temperature = 70.3;
  testData.humidity = 34.3;
  testData.particle_2_5_count = 25;
  testData.voc_data = 0;
  testData.co2_data = 0;

  pinDataCharacteristic.write(&testData, sizeof(wearable_data_t));
}

void loop() 
{
#ifdef PIN_SERIAL_ON
#endif
  wearable_data_t wd = {70.9, 53.1, 12, 124, 23233};
  q.enqueue(wd);
  delay(5000);
  wd = q.dequeue();
}
