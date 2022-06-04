#include <bluefruit.h>
#include <DS3232RTC.h>
#include <SPI.h>
#include <SdFat.h>
#include <Adafruit_SPIFlash.h>
#include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h>  // Fuel gauge
#include "Adafruit_MPRLS.h" //pressure sensor
#include <arduino-timer.h>
#include "IUE_t.h"
#include "iueQueue.h"

#include "inhalerDebug.h"


#define IUE_PIN 6
#define BLE_PIN 5
#define R_LED_PIN 16
#define G_LED_PIN 15
#define B_LED_PIN 14
#define NEBULIZER_PIN 9
#define BLE_LED_INDICATION_TIME 5000
#define IUE_TIME 5000 //Total nebulization running time during IUE
#define IUE_TIMEOUT 15000
#define INDICATION_TIMEOUT 5000
#define PRESSURE_DELTA -50 //Pressure difference for IUE; negative is suck, positive is blow
#define BLE_FAST_TIMEOUT 30 //amount of seconds advertising in fast mode, set to 0 for continuous
#define BLE_ADV_DEFAULT_LENGTH 60 //amount of seconds advertising, set to 0 for continuous
#define BLE_BLINK_INTERVAL 1000

//functions that have default parameters must be explicitly prototyped in arduino
void bleDisconnectHandler(uint16_t conn_hdl=0, uint8_t reason=0);
void bleConnectionHandler(uint16_t conn_hdl=0);
void startBleAdvertising(int seconds = BLE_ADV_DEFAULT_LENGTH);

#ifdef RTC_CONNECTED
DS3232RTC RTC;
#endif

#ifdef PRESSURE_SENSOR
Adafruit_MPRLS mpr;
#endif

#ifdef LIPO_CONNECTED
SFE_MAX1704X lipo(MAX1704X_MAX17048);
#endif

//Inhaler UUIDs
#define INHALER_SERVICE_UUID              "e814c25d7107459eb25d23fec96d49da"
#define INHALER_TIME_CHARACTERISTIC_UUID  "015529f7554c4138a71e40a2dfede10a"
#define INHALER_IUE_CHARACTERISTIC_UUID   "d7dc7c5048ce45a49c3e243a5bb75608"
#define INHALER_APPEARANCE 0x03C0 //BLE Apperance code for human interface device

int Pressure_Ref = 0;
unsigned long bleBlinkStartTime = 0;
int bleLedToggleCount = 0;

iueQueue q;

Timer<4> timer; //up to four timers at once
uintptr_t bleBlinkTask;
uintptr_t endBleBlinkTask;

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
volatile bool iueTriggered = false;
volatile bool bleTriggered = false;
bool bleLightOn = false;

void setup()
{
  //#ifdef INHALER_SERIAL_ON
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Serial Connected");
  Serial.flush();
  //#endif
  /*
     PIN SETUP
  */
  pinMode(IUE_PIN, INPUT_PULLUP);
  pinMode(BLE_PIN, INPUT_PULLUP);
  pinMode(NEBULIZER_PIN, OUTPUT);
  digitalWrite(NEBULIZER_PIN, LOW);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(R_LED_PIN, OUTPUT);
  pinMode(G_LED_PIN, OUTPUT);
  pinMode(B_LED_PIN, OUTPUT);
  resetLEDs(NULL);

  attachInterrupt(digitalPinToInterrupt(IUE_PIN), setIueTriggered, RISING);
  attachInterrupt(digitalPinToInterrupt(BLE_PIN), setBleTriggered, RISING);
#ifdef LIPO_CONNECTED
  while (!lipo.begin());
#endif

#ifdef PRESSURE_SENSOR
  while (!mpr.begin());
#endif

  /*
     RTC SETUP
  */
#ifdef RTC_CONNECTED
  RTC.begin();
  //setSyncProvider(RTC.get);
#ifdef INHALER_SERIAL_ON
  if (timeStatus() != timeSet)
    Serial.println(F("Unable to sync with the RTC"));
  else
    Serial.println(F("RTC has set the system time"));
#endif
  setRTCSerial();
#endif

#ifdef LIPO_CONNECTED
  lipo.setThreshold(30); //set low-battery alert to 30% per proposal specification
  delay(100);
#endif

#ifdef PRESSURE_SENSOR
  Pressure_Ref = (int) mpr.readPressure();
#endif

  /*
     QUEUE SETUP
  */
  q.begin();

  /*
     BLEDIS setup
  */
  bledis.setModel("Bluefruit Feather52");
  bledis.begin();

  /*
     INHALER BLE SETUP
  */
  Bluefruit.autoConnLed(false); //turn off on board BLE LED

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
     BLE Advertising Setup
  */
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addUuid(inhalerServiceUuid); //android app looks for service uuid in advertising
  Bluefruit.Advertising.addService(inhalerService);
  Bluefruit.Advertising.addAppearance(INHALER_APPEARANCE);
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.restartOnDisconnect(false);
  Bluefruit.Advertising.setFastTimeout(BLE_FAST_TIMEOUT);
  //Bluefruit.Advertising.setStopCallback(bleDisconnectHandler);
  Bluefruit.Periph.setConnectCallback(bleConnectionHandler);
  Bluefruit.Periph.setDisconnectCallback(bleDisconnectHandler);
  Bluefruit.Periph.begin();
  startBleAdvertising();



  /*
     NON BLE SETUP
  */
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
  timer.tick();
  if (iueTriggered)
  {
    handleIUE();
    iueTriggered = false;
  }
  if (bleTriggered)
  {
#ifdef INHALER_SERIAL_ON
    Serial.println("BLE Button Pressed");
#endif
    startBleAdvertising();
    bleTriggered = false;
  }
}

void handleIUE()
{
#ifdef INHALER_SERIAL_ON
  Serial.println(F("Recieved IUE"));
#endif

  resetLEDs(NULL);
  batteryState();
  bool nebulizerActivated = false;

#ifdef PRESSURE_SENSOR
  int treatMillis = 0;
  unsigned long startTime = millis();
  unsigned long IUE_Accumulator = startTime;
  while (treatMillis < IUE_TIME)
  {
    int currentPressureDelta = (int) mpr.readPressure() - Pressure_Ref;
    int currentMillis = millis();

    if (currentMillis - startTime > IUE_TIMEOUT)
    {
#ifdef INHALER_SERIAL_ON
      Serial.println("Treatment Phase Timing Out!");
#endif
      break;
    }

    if (currentPressureDelta <= PRESSURE_DELTA) //if pressure sensor shows use
    {
      digitalWrite(NEBULIZER_PIN, HIGH);
      nebulizerActivated = true;
      int deltaMillis = currentMillis - IUE_Accumulator;
      if (deltaMillis >= 50)
      {
        treatMillis += deltaMillis;
        IUE_Accumulator = currentMillis;
      }
    }
    else
    {
      digitalWrite(NEBULIZER_PIN, LOW);
      IUE_Accumulator = currentMillis;
    }
  }
  digitalWrite(NEBULIZER_PIN, LOW);
#else
  digitalWrite(NEBULIZER_PIN, HIGH);
  delay(5000);
  digitalWrite(NEBULIZER_PIN, LOW);
#endif

  if (nebulizerActivated)
  {
    resetLEDs(NULL);
    analogWrite(B_LED_PIN, 100);
    timer.in(BLE_LED_INDICATION_TIME, resetLEDs);
    IUE_t iue;
#ifdef RTC_CONNECTED
    iue.timestamp = getTime() * 1000; //multiply by 1000 because the app reqires milliseconds
#else
    iue.timestamp = 0x180A1AE1D03 + millis();
#endif
    q.enqueue(iue);
    sendIUEs();
  }
  else
    resetLEDs(NULL);
}

void sendIUEs()
{
  while (!q.empty() && Bluefruit.connected(Bluefruit.connHandle()))
  {
    IUE_t iue = q.dequeue();
    bool indicationSuccessful = false;
    unsigned long startTime = millis();
    while (!indicationSuccessful && millis() - startTime < INDICATION_TIMEOUT)
    {
      indicationSuccessful = inhalerIueCharacteristic.indicate(&iue, sizeof(IUE_t));
#ifdef INHALER_SERIAL_ON
      if (indicationSuccessful)
        Serial.println(F("Indication Sent!"));
      else
        Serial.println(F("Indication not Successful"));
#endif
    }
#ifdef INHALER_SERIAL_ON
    if (!indicationSuccessful)
      Serial.println("Indication Timed Out!");
#endif
  }
}


void batteryState()
{
#ifdef LIPO_CONNECTED
  int s = (int) lipo.getSOC();
  if (s >= 70)
    analogWrite(G_LED_PIN, 255);
  else if ((s > 30) && (s < 70))
  {
    analogWrite(R_LED_PIN, 70);
    analogWrite(G_LED_PIN, 30);
  }
  else
    analogWrite(R_LED_PIN, 100);
#else
  analogWrite(G_LED_PIN, 255);
#endif
}

bool resetLEDs(void*)
{
  analogWrite(R_LED_PIN, 0);
  analogWrite(G_LED_PIN, 0);
  analogWrite(B_LED_PIN, 0);
  return false;
}

void bleDisconnectHandler(uint16_t conn_hdl, uint8_t reason)
{
#ifdef INHALER_SERIAL_ON
  Serial.println("In BLE Disconnect Handler");
#endif
  startBleAdvertising();
}

void startBleAdvertising(int seconds)
{
  if(Bluefruit.Advertising.isRunning())
    Bluefruit.Advertising.stop();
  Bluefruit.Advertising.start(seconds);
  startBleLedBlink(seconds);
}

//this fucntion is meant to be used with a timer to toggle the BLE LED every
//second, the function signals for the timer to end when it has toggled
bool toggleBleLed(void*)
{
  bleLightOn = !bleLightOn;
  bleLedToggleCount++;
  if(bleLightOn)
    analogWrite(B_LED_PIN, 255);
  else
    analogWrite(B_LED_PIN, 0);
    
#ifdef INHALER_SERIAL_ON
    Serial.print(bleLedToggleCount);
    Serial.print(": Toggled LED\n");
#endif
    return true;
}

void startBleLedBlink(int seconds)
{
  //reset blink if already going
  timer.cancel(bleBlinkTask);
  timer.cancel(endBleBlinkTask);
  turnOffBleLed(NULL);
  bleLedToggleCount = 0;

  //start blink
  bleBlinkTask = timer.every(BLE_BLINK_INTERVAL, toggleBleLed);
  endBleBlinkTask = timer.in(seconds*1000, endBleLedBlink);
}

bool endBleLedBlink(void*)
{
#ifdef INHALER_SERIAL_ON
  Serial.println("Ending BLE LED Blink");
#endif
  timer.cancel(endBleBlinkTask);
  timer.cancel(bleBlinkTask);
  turnOffBleLed(NULL);
  return false;
}

void bleConnectionHandler(uint16_t conn_hdl)
{
#ifdef INHALER_SERIAL_ON
  Serial.println("In BLE Connection Handler");
#endif
  endBleLedBlink(NULL);
  turnOnBleLed();
  timer.in(10000, turnOffBleLed);
  sendIUEs();
}

void turnOnBleLed()
{
  analogWrite(B_LED_PIN, 255);
}

bool turnOffBleLed(void*)
{
  analogWrite(B_LED_PIN, 0);
  return false;
}

/*
   IUE interrupt routine
*/
void setIueTriggered()
{
  iueTriggered = true;
}

/*
   BLE interrupt routine
*/
void setBleTriggered()
{
  if (!iueTriggered)
    bleTriggered = true;
}

/*
   Gets the time from the RTC in epoch seconds and returns it as a time_t
*/
#ifdef RTC_CONNECTED
time_t getTime()
{
  uint64_t t;
  tmElements_t tm;
  int8_t retVal = RTC.read(tm);
#ifdef INHALER_SERIAL_ON
  if (retVal != 0)
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
  Serial.println(F("Enter UTC Date and time yy,m,d,h,m,s"));
  while (!Serial.available());
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
