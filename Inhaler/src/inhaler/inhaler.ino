#include <bluefruit.h>
#include <time.h>

typedef struct{
  int64_t timestamp;      // 8 bytes
} IUE_t;

//Inhaler UUIDs
#define INHALER_SERVICE_UUID              "e814c25d7107459eb25d23fec96d49da"
#define INHALER_TIME_CHARACTERISTIC_UUID  "015529f7554c4138a71e40a2dfede10a"
#define INHALER_IUE_CHARACTERISTIC_UUID   "d7dc7c5048ce45a49c3e243a5bb75608"
#define INHALER_APPEARANCE 0x03C0 //BLE Apperance code for human interface device

//setup Inhaler UUIDs
BLEUuid inhalerServiceUuid(INHALER_SERVICE_UUID);
BLEUuid inhalerTimeCharacteristicUuid(INHALER_TIME_CHARACTERISTIC_UUID);
BLEUuid inhalerIueCharacteristicUuid(INHALER_IUE_CHARACTERISTIC_UUID);

//create service and characteristic classes
BLEService inhalerService(inhalerServiceUuid);
BLECharacteristic inhalerTimeCharacteristic(inhalerTimeCharacteristicUuid);
BLECharacteristic inhalerIueCharacteristic(inhalerIueCharacteristicUuid);

//create test IUE
IUE_t iueTest;

void setup() 
{
  //test sending IUE
  attachInterrupt(digitalPinToInterrupt(7), sendIUE, RISING);
  
  /*
   * BLEDIS setup
   */
   BLEDis bledis;
   bledis.setModel("Bluefruit Feather52");
   bledis.begin();

  /*
   * INHALER BLE SETUP
   */
  //setup Inhaler BLE Service
  

  //setup Inhaler BLE Characteristics
  inhalerTimeCharacteristic.setProperties(CHR_PROPS_READ);
  inhalerTimeCharacteristic.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
    
  inhalerIueCharacteristic.setProperties(CHR_PROPS_INDICATE); //has to be set to indicate for app to allow transfer
  inhalerIueCharacteristic.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);

  //initialize ble settings
  Bluefruit.setName("Smart Inhaler");
  Bluefruit.begin();

  //start inhaler service and attach characteristic
  inhalerService.begin();
  inhalerIueCharacteristic.begin();
  
  /*
   * BLE Advertising Setup
   */
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addUuid(inhalerServiceUuid); //android app looks for service uuid in advertising
  Bluefruit.Advertising.addService(inhalerService);
  Bluefruit.Advertising.addAppearance(INHALER_APPEARANCE);
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

  //create test data
  //iueTest.timestamp = 1649569555; //April 9, 2022
  iueTest.timestamp = 0x0000000099999999;
}

void loop() 
{
 /*
  //check for connection
  if(Bluefruit.connected(Bluefruit.connHandle()))
  {
    Serial.println("CONNECTED");
    BLEConnection* con = Bluefruit.Connection(Bluefruit.connHandle()); 
    Serial.print("Role: ");
    Serial.println(con->getRole());
    Serial.print("Con Interval: ");
    Serial.println(con->getConnectionInterval());
  }
  delay(1000);
  */
  
  
  

}

void sendIUE()
{
  Serial.println("Recieved Interrupt");
  iueTest.timestamp += 10;
  
  int8_t* iue_ptr = (int8_t*) &iueTest;
  Serial.print("sending IUE: ");
  for(int i = 0; i < sizeof(IUE_t); i++)
  {
    Serial.print("<");
    Serial.print(iue_ptr[i]);
    Serial.print(">");
  }
  Serial.println();

  if(Bluefruit.connected(Bluefruit.connHandle()))
    inhalerIueCharacteristic.indicate(&iueTest, sizeof(IUE_t));
  
}
