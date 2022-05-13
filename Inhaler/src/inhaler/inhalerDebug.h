#ifndef INHALER_DEBUG
#define INHALER_DEBUG

#define INHALER_SERIAL_ON

static void probe(int p)
{
#ifdef INHALER_SERIAL_ON
  Serial.print("PROBE ");
  Serial.println(p);
  Serial.flush();
#endif
}
#endif
