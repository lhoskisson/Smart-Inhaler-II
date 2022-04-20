#include <bluefruit.h>

#define PIN_SERIAL_ON

typedef struct{
  float temperature;            // 4 bytes - little endian
  float humidity;               // 4 bytes - little endian
  int32_t  particle_0_5_count;  // 4 bytes - little endian
  char character;               // 1 byte
  char  digit;                  // 1 byte
} wearable_data_t;

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
#endif

  /*
   * BLEDIS setup
   */
   bledis.setModel("Bluefruit Feather52");
   bledis.begin();
  
  /*
   * PIN BLE SETUP
   */
  pinDataCharacteristic.setProperties(CHR_PROPS_READ);
  pinDataCharacteristic.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  pinDataCharacteristic.setFixedLen(sizeof(wearable_data_t));

  //start pin service, and attach charactersitic
  pinService.begin();
  pinDataCharacteristic.begin();

  Serial.println("Started Services and Characteristics");
  
  /*
   * BLE Advertising Setup
   */
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addUuid(pinServiceUuid); //android app looks for service uuid in advertising
  Bluefruit.Advertising.addService(pinService);
  Bluefruit.Advertising.addAppearance(PIN_APPEARANCE);
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.start();

  Serial.println("Started Advertising");
  
  /*
   * NON BLE SETUP
   */


  //creating test data
  testData.temperature = 70.3;
  testData.humidity = 16.4;
  testData.particle_0_5_count = 542;
  testData.character = 'f';
  testData.digit = 42;

  //pinDataCharacteristic.write(&testData, sizeof(wearable_data_t));
}

void loop() 
{
  Serial.println("In Loop");
  delay(5000);
}
