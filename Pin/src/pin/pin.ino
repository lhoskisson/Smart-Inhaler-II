#include <bluefruit.h>

typedef struct{
  float temperature;            // 4 bytes - little endian
  float humidity;               // 4 bytes - little endian
  int32_t  particle_0_5_count;  // 4 bytes - little endian
  char character;               // 1 byte
  char  digit;                  // 1 byte
} wearable_data_t;

#define PIN_DATA_LENGTH (sizeof(wearable_data_t))

//Pin UUIDs
#define PIN_SERVICE_UUID                  "25380284e1b6489abbcf97d8f7470aa4"
#define PIN_DATA_CHARACTERISTIC_UUID      "c3856cfa4af64d0da9a05ed875d937cc"
#define PIN_APPEARANCE 0x0552   //BLE Apperance code for multi-sensor

void setup() 
{
  /*
   * BLEDIS setup
   */
   BLEDis bledis;
   bledis.setModel("Bluefruit Feather52");
   bledis.begin();
  
  /*
   * PIN BLE SETUP
   */
  //setup Pin UUIDs
  BLEUuid pinServiceUuid(PIN_SERVICE_UUID);
  BLEUuid pinDataCharacteristicUuid(PIN_DATA_CHARACTERISTIC_UUID);
  
  //setup Pin BLE Service
  BLEService pinService(pinServiceUuid);

 //setup Pin BLE Characteristics
  BLECharacteristic pinDataCharacteristic(pinDataCharacteristicUuid);
  pinDataCharacteristic.setProperties(CHR_PROPS_READ);
  pinDataCharacteristic.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  pinDataCharacteristic.setFixedLen(PIN_DATA_LENGTH);

  /*
   * BLE Advertising Setup
   */
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addUuid(pinServiceUuid); //android app looks for service uuid in advertising
  Bluefruit.Advertising.addService(pinService);
  Bluefruit.Advertising.addAppearance(PIN_APPEARANCE);
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.start();

  //start pin service, and attach charactersitic
  pinService.begin();
  pinDataCharacteristic.begin();


  /*
   * NON BLE SETUP
   */
  pinMode(LED_BUILTIN, OUTPUT);

  //setup serial connection for debug
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Serial Connected");
}

void loop() 
{

}
