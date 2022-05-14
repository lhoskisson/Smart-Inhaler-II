#ifndef PIN_DEBUG
#define PIN_DEBUG

#define PIN_SERIAL_ON

static void probe(int p)
{
#ifdef PIN_SERIAL_ON
  Serial.print("PROBE ");
  Serial.println(p);
  Serial.flush();
#endif
}
#endif
