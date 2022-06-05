#ifndef INHALER_DEBUG
#define INHALER_DEBUG

//#define INHALER_SERIAL_ON
#define PRESSURE_SENSOR
#define LIPO_CONNECTED
#define RTC_CONNECTED

static void beginSerial()
{
#ifdef INHALER_SERIAL_ON
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Serial Connected");
  Serial.flush();
#endif
}
static void probe(int p)
{
#ifdef INHALER_SERIAL_ON
  Serial.print("PROBE ");
  Serial.println(p);
  Serial.flush();
#endif
}
#endif
