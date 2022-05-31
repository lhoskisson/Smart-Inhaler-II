#ifndef PIN_DEBUG
#define PIN_DEBUG

//#define PIN_SERIAL_ON
//#define LIPO_CONNECTED
#define TEMPERATURE_SENSOR_CONNECTED
#define VOC_SENSOR_CONNECTED
#define PM_SENSOR_CONNECTED

static void probe(int p)
{
#ifdef PIN_SERIAL_ON
  Serial.print("PROBE ");
  Serial.println(p);
  Serial.flush();
#endif
}
#endif
