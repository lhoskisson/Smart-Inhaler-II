#include <bluefruit.h>
#include "wearable_data_t.h"
#include "wearableDataQueue.h"
#include <SPI.h>
#include <Wire.h>
#include <SdFat.h>
#include <Adafruit_SPIFlash.h>
#include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h>  // Fuel gauge
#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_PM25AQI.h>
#include <Adafruit_SGP30.h>
#include <Adafruit_Sensor.h>
#include <arduino-timer.h>

#include "pinDebug.h"

#define DATA_COLLECTION_INTERVAL 10000//600000 //10min
#define SGP30_BASELINE_CALIBRATION_TIME 43200000 //12 hours
#define SGP30_BASELINE_COLLECTION_INTERVAL 3600000 //1 hour
#define SGP30_BASELINE_FILENAME "baseline"
#define FAN_ON_INTERVAL 30000 //30 seconds
#define FAN_ON_TIME 10000 //10 seconds
#define FAN_OFF_INTERVAL (FAN_ON_INTERVAL+FAN_ON_TIME)
#define PM25_PIN 9
#define FAN_PIN 11

bool sgp30BaselineCalibrated = false;

//4 timers: fan on, fan off, data collection, and SGP30 baseline recording
Timer<4> timer;

wearableDataQueue q;

FatFileSystem fs;
Adafruit_FlashTransport_QSPI flashTransport;
Adafruit_SPIFlash flash(&flashTransport);

#ifdef LIPO_CONNECTED
SFE_MAX1704X lipo(MAX1704X_MAX17048);
#endif

Adafruit_PM25AQI aqi = Adafruit_PM25AQI();
PM25_AQI_Data pmData; //PM2.5 sensor

#ifdef VOC_SENSOR_CONNECTED
Adafruit_SGP30 sgp; //VOC SGP30 sensor
#endif

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

  pinMode(FAN_PIN, OUTPUT); //set digital pin 11 as output
  pinMode(PM25_PIN,OUTPUT); //set pin 9 to be an output output
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(PM25_PIN, HIGH);
  
  fanOff(NULL);
  
  //setupt flash and file system
  while(!flash.begin());
  while(!fs.begin(&flash));

  /*
   * QUEUE SETUP
   */
  q.begin();
 
  timer.every(DATA_COLLECTION_INTERVAL, recordWearableData);
  timer.every(FAN_ON_INTERVAL, fanOn);
  timer.every(FAN_OFF_INTERVAL, fanOff);
  timer.every(SGP30_BASELINE_COLLECTION_INTERVAL, recordSgp30Baseline);
  
#ifdef LIPO_CONNECTED
  lipo.quickStart();
  lipo.setThreshold(10);
#endif

#ifdef TEMPERATURE_SENSOR_CONNECTED
  dht.begin();
#endif

#ifdef VOC_SENSOR_CONNECTED
  sgp.begin();
  //attempt to get stored baseline
  uint16_t eco2_base;
  uint16_t tvoc_base;
  if(readSgp30Baseline(&eco2_base, &tvoc_base))
  {
#ifdef PIN_SERIAL_ON
    Serial.println("Read SPG30 Baseline");
    Serial.print("eCO2 Baseline: ");
    Serial.println(eco2_base);
    Serial.print("TOVC Baseline ");
    Serial.println(tvoc_base);
#endif
    sgp.setIAQBaseline(eco2_base, tvoc_base);
    sgp30BaselineCalibrated = true;
  }
#endif

#ifdef PM_SENSOR_CONNECTED
  aqi.begin_I2C();
#endif
  
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
  timer.tick();



#ifdef LIPO_CONNECTED
#ifdef PIN_SERIAL_ON
  voltage = lipo.getVoltage();
  soc = lipo.getSOC();
  alert = lipo.getAlert();
  //Print out battery satus
  //Serial.print("voltage"); Serial.print(voltage);Serial.println();
  //Serial.print("state of charge"); Serial.print(soc);Serial.println();
  //Serial.print("alert"); Serial.print(alert);Serial.println();
#endif
#endif
}

wearable_data_t getWearableData()
{
  wearable_data_t wd;
#ifdef TEMPERATURE_SENSOR_CONNECTED
  wd.temperature = dht.readTemperature();
  wd.humidity = dht.readHumidity();
#endif

  //callibrate SGP30 based on temperature and humidity
  sgp.setHumidity(getAbsoluteHumidity(wd.temperature,wd.humidity));

#ifdef VOC_SENSOR_CONNECTED
  sgp.IAQmeasure();
  wd.voc_data = sgp.TVOC;
  wd.co2_data = sgp.eCO2;
#endif

#ifdef PM_SENSOR_CONNECTED
  aqi.read(&pmData);
  wd.particle_2_5_count = pmData.pm25_standard;
  wd.particle_10_count = pmData.pm10_standard;
#endif

#ifdef PIN_SERIAL_ON
  Serial.println("In getWearableData");
  printWearableData(wd);
#endif
  return wd;
}

bool recordWearableData(void*)
{
  wearable_data_t currentData = getWearableData();
  pinDataCharacteristic.write(&currentData, sizeof(wearable_data_t)); //todo: remove this when queue upload has been implemented on app
  q.enqueue(currentData);
  return true;
}

bool recordSgp30Baseline(void*)
{
  if(!sgp30BaselineCalibrated && millis() < SGP30_BASELINE_CALIBRATION_TIME)
    return true;
    
  uint16_t eco2_base = 0;
  uint16_t tvoc_base = 0;
  sgp.getIAQBaseline(&eco2_base, &tvoc_base);
  if(fs.exists(SGP30_BASELINE_FILENAME))
    fs.remove(SGP30_BASELINE_FILENAME);
  File baselineFile = fs.open(SGP30_BASELINE_FILENAME, FILE_WRITE);
  baselineFile.write(&eco2_base, sizeof(uint16_t));
  baselineFile.write(&tvoc_base, sizeof(uint16_t));
  baselineFile.close();
  return true;
}

bool readSgp30Baseline(uint16_t* eco2_base, uint16_t* tvoc_base)
{

  if(!fs.exists(SGP30_BASELINE_FILENAME))
    return false;

  File baselineFile = fs.open(SGP30_BASELINE_FILENAME, FILE_READ);
  
  if(baselineFile.size() != (2*sizeof(uint16_t)))
  {
    baselineFile.close();
    return false;
  }

  baselineFile.read(eco2_base, sizeof(uint16_t));
  baselineFile.read(tvoc_base, sizeof(uint16_t));
  baselineFile.close();
  return true;
}

bool fanOn(void*)
{
#ifdef PIN_SERIAL_ON
  Serial.println("Turning Fans on");
#endif
  //digitalWrite(PM25_PIN, HIGH);
  digitalWrite(FAN_PIN, HIGH);
#ifdef PM_SENSOR_CONNECTED
  aqi.begin_I2C();
#endif
  return true;
}

bool fanOff(void*)
{
#ifdef PIN_SERIAL_ON
  Serial.println("Turning Fans off");
#endif
  //digitalWrite(PM25_PIN, LOW);
  digitalWrite(FAN_PIN, LOW);
  return true;
}

uint32_t getAbsoluteHumidity(float temperature, float humidity)
{
    // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
    return absoluteHumidityScaled;
}
