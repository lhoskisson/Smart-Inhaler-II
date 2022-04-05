#include <bluefruit.h>

typedef struct{
  int64_t timestamp;      // 8 bytes
} IUE_t;

//Inhaler UUIDs
#define INHALER_SERVICE_UUID              "e814c25d7107459eb25d23fec96d49da"
#define INHALER_TIME_CHARACTERISTIC_UUID  "015529f7554c4138a71e40a2dfede10a"
#define INHALER_IUE_CHARACTERISTIC_UUID   "d7dc7c5048ce45a49c3e243a5bb75608"
#define INHALER_APPEARANCE 0x03C0 //BLE Apperance code for human interface device

void setup() 
{
  /*
   * BLEDIS setup
   */
   BLEDis bledis;
   bledis.setModel("Bluefruit Feather52");
   bledis.begin();

  /*
   * INHALER BLE SETUP
   */
  //setup Inhaler UUIDs
  BLEUuid inhalerServiceUuid(INHALER_SERVICE_UUID);
  BLEUuid inhalerTimeCharacteristicUuid(INHALER_TIME_CHARACTERISTIC_UUID);
  BLEUuid inhalerIueCharacteristicUuid(INHALER_IUE_CHARACTERISTIC_UUID);

  //setup Inhaler BLE Service
  BLEService inhalerService(inhalerServiceUuid);

  //setup Inhaler BLE Characteristics
  BLECharacteristic inhalerTimeCharacteristic(inhalerTimeCharacteristicUuid);
  inhalerTimeCharacteristic.setProperties.(CHR_PROPS_READ);
  inhalerTimeCharacteristic.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  
  BLECharacteristic inhalerIueCharacteristic(inhalerIueCharacteristicUuid);
  inhalerIueCharacteristic.setProperties.(CHR_PROPS_READ);
  inhalerIueCharacteristic.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  
  Bluefruit.setName("Smart_Pin");
  Bluefruit.begin();

  /*
   * BLE Advertising Setup
   */
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addService(pinService);
  Bluefruit.Advertising.addAppearance(PIN_APPEARANCE);
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.start();

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
