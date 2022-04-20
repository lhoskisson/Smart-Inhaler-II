#include <bluefruit.h>

//#define INHALER_SERIAL_ON

typedef struct{
  int64_t timestamp;      // 8 bytes
} IUE_t;

//Inhaler UUIDs
#define INHALER_SERVICE_UUID              "e814c25d7107459eb25d23fec96d49da"
#define INHALER_TIME_CHARACTERISTIC_UUID  "015529f7554c4138a71e40a2dfede10a"
#define INHALER_IUE_CHARACTERISTIC_UUID   "d7dc7c5048ce45a49c3e243a5bb75608"
#define INHALER_APPEARANCE 0x03C0 //BLE Apperance code for human interface device

//setup Inhaler UUID classes
BLEUuid inhalerServiceUuid(INHALER_SERVICE_UUID);
BLEUuid inhalerTimeCharacteristicUuid(INHALER_TIME_CHARACTERISTIC_UUID);
BLEUuid inhalerIueCharacteristicUuid(INHALER_IUE_CHARACTERISTIC_UUID);

//create service and characteristic classes
BLEService inhalerService(inhalerServiceUuid);
BLECharacteristic inhalerTimeCharacteristic(inhalerTimeCharacteristicUuid);
BLECharacteristic inhalerIueCharacteristic(inhalerIueCharacteristicUuid);

BLEDis bledis;

//create test IUE
volatile IUE_t iueTest;

//flags
bool iueTriggered = false;

void setup() 
{
  //test sending IUE
  attachInterrupt(digitalPinToInterrupt(7), setIueTriggered, RISING);
  
  /*
   * BLEDIS setup
   */
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
  inhalerIueCharacteristic.setFixedLen(sizeof(IUE_t));

/*
  iueTest.timestamp = 0;
  inhalerIueCharacteristic.indicate(&iueTest, sizeof(IUE_t));
  */

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

#ifdef INHALER_SERIAL_ON
  //setup serial connection for debug
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Serial Connected");
#endif

  //create test data
  //iueTest.timestamp = 1649824988*1000;
  iueTest.timestamp = 1650407841131;
  //iueTest.timestamp = 0x00000000FFFFFFFF;
  //iueTest.timestamp = 0xFFFFFFFF00000000;
}

void loop() 
{
  if(iueTriggered)
  {
    sendIUE();
    iueTriggered = false;
  }
  /*
  iueTest.timestamp += 5;
  if(inhalerIueCharacteristic.indicate(&iueTest, sizeof(IUE_t)))
    Serial.println("Indication Sent!");
  delay(5000);
  */
  
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
  IUE_t iue;
  iue.timestamp = iueTest.timestamp;

#ifdef INHALER_SERIAL_ON
  Serial.println("Recieved Interrupt");
  
  int8_t* iue_ptr = (int8_t*) &iueTest;
  Serial.print("sending IUE: ");
  for(int i = sizeof(IUE_t)-1; i != 0; i--)
  {
    Serial.print("<");
    Serial.print(iue_ptr[i], HEX);
    Serial.print(">");
  }
  Serial.println();
#endif

  if(Bluefruit.connected(Bluefruit.connHandle()))
  {
    bool retVal = inhalerIueCharacteristic.indicate(&iue, sizeof(IUE_t));
    
#ifdef INHALER_SERIAL_ON
    if(retVal)
      Serial.println("Indication Sent!");
#endif
  }
}

void setIueTriggered()
{
  iueTriggered = true;
  iueTest.timestamp += 16;
}
