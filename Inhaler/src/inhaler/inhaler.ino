#include <bluefruit.h>
#include <DS3232RTC.h>
//#include <Streaming.h>
#include <SPI.h>
#include <SdFat.h>
#include <Adafruit_SPIFlash.h>
#include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h>  // Fuel gauge
#include "Adafruit_MPRLS.h" //pressure sensor
#include "IUE_t.h"
#include "iueQueue.h"

#include "inhalerDebug.h"

#define IUE_PIN 7
#define BLE_PIN
//#define RTC_CONNECTED

#ifdef RTC_CONNECTED
DS3232RTC RTC;
#endif

//Inhaler UUIDs
#define INHALER_SERVICE_UUID              "e814c25d7107459eb25d23fec96d49da"
#define INHALER_TIME_CHARACTERISTIC_UUID  "015529f7554c4138a71e40a2dfede10a"
#define INHALER_IUE_CHARACTERISTIC_UUID   "d7dc7c5048ce45a49c3e243a5bb75608"
#define INHALER_APPEARANCE 0x03C0 //BLE Apperance code for human interface device

iueQueue q;

//setup Inhaler UUID classes
BLEUuid inhalerServiceUuid(INHALER_SERVICE_UUID);
BLEUuid inhalerTimeCharacteristicUuid(INHALER_TIME_CHARACTERISTIC_UUID);
BLEUuid inhalerIueCharacteristicUuid(INHALER_IUE_CHARACTERISTIC_UUID);

//create service and characteristic classes
BLEService inhalerService(inhalerServiceUuid);
BLECharacteristic inhalerTimeCharacteristic(inhalerTimeCharacteristicUuid);
BLECharacteristic inhalerIueCharacteristic(inhalerIueCharacteristicUuid);

BLEDis bledis;

//flags
bool iueTriggered = false;

void setup() 
{

#ifdef INHALER_SERIAL_ON
  //setup serial connection for debug
  Serial.begin(115200);
  while(!Serial);
  Serial.println(F("Serial Connected"));
#endif
  probe(1);
/*
 * QUEUE SETUP
 */
 q.begin();
 
  //test sending IUE
  attachInterrupt(digitalPinToInterrupt(IUE_PIN), setIueTriggered, RISING);
  
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



 /*
  * RTC SETUP
  */
#ifdef RTC_CONNECTED
  RTC.begin();
  //setSyncProvider(RTC.get);
#ifdef INHALER_SERIAL_ON
  if(timeStatus() != timeSet)
    Serial.println(F("Unable to sync with the RTC"));
  else
    Serial.println(F("RTC has set the system time"));
#endif
  setRTCSerial();
#endif
}

void loop() 
{
  if(iueTriggered)
  {
    sendIUE();
    iueTriggered = false;
  }
}

void sendIUE()
{
  IUE_t iue;
#ifdef RTC_CONNECTED
  iue.timestamp = getTime()*1000; //multiply by 1000 because the app reqires milliseconds
#else
  iue_count += 10000;
  iue.timestamp = 0x180A1AE1D03 + iue_count;
#endif

#ifdef INHALER_SERIAL_ON
  Serial.println(F("Recieved Interrupt"));
  printIUE(iue);
  Serial.println();
#endif

  q.enqueue(iue);

  //if there is a BLE connection, the IUE queue is uploaded
  if(Bluefruit.connected(Bluefruit.connHandle()))
  {
    while(!q.empty())
    {
      iue = q.dequeue();
      bool retVal = inhalerIueCharacteristic.indicate(&iue, sizeof(IUE_t));
#ifdef INHALER_SERIAL_ON
      if(retVal)
        Serial.println(F("Indication Sent!"));
#endif
    } 
  }
#ifdef INHALER_SERIAL_ON
  Serial.println("");
#endif   
}

/*
 * IUE interrupt routine
 */
void setIueTriggered()
{
  iueTriggered = true;
}

/*
 * Gets the time from the RTC in epoch seconds and returns it as a time_t 
 */
#ifdef RTC_CONNECTED
time_t getTime()
{
  uint64_t t;
  tmElements_t tm;
  int8_t retVal = RTC.read(tm);
#ifdef INHALER_SERIAL_ON
  if(retVal != 0)
    Serial.println(F("Could not read time from RTC"));
#endif
  return makeTime(tm);
}
#endif

#ifdef RTC_CONNECTED
void setRTCSerial()
{
  //EXAMPLE CODE FROM RTC Library to set RTC with Serial input
#ifdef INHALER_SERIAL_ON
    time_t t;
    tmElements_t tm;
// check for input to set the RTC, minimum length is 12, i.e. yy,m,d,h,m,s
    Serial.println(F("Enter Date and time yy,m,d,h,m,s"));
    while(!Serial.available());
    if (Serial.available() >= 12) {
        // note that the tmElements_t Year member is an offset from 1970,
        // but the RTC wants the last two digits of the calendar year.
        // use the convenience macros from the Time Library to do the conversions.
        int y = Serial.parseInt();
        if (y >= 100 && y < 1000)
          Serial.println(F("Error: Year must be two digits or four digits!"));
        else {

            if (y >= 1000)
                tm.Year = CalendarYrToTm(y);
            else    // (y < 100)
                tm.Year = y2kYearToTm(y);
            tm.Month = Serial.parseInt();
            tm.Day = Serial.parseInt();
            tm.Hour = Serial.parseInt();
            tm.Minute = Serial.parseInt();
            tm.Second = Serial.parseInt();
            t = makeTime(tm);
            RTC.set(t);   // use the time_t value to ensure correct weekday is set
            // dump any extraneous input
            while (Serial.available() > 0) Serial.read();
        }
    }
    Serial.println(F("leaving setRTC"));
    Serial.flush();
#endif
}
#endif
