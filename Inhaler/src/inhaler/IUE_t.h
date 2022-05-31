#ifndef IUE_T_H
#define IUE_T_H

#include "Arduino.h"
#include <time.h>

#include "inhalerDebug.h"

typedef struct{
  uint64_t timestamp;      // 8 bytes
} IUE_t;

#ifdef INHALER_SERIAL_ON
static void printIUE(IUE_t iue)
{
  iue.timestamp /= 1000; //convert back to seconds
  int8_t* iue_ptr = (int8_t*) &iue;
  Serial.print(F("IUE (UTC): "));
  Serial.print(ctime((const time_t*) &iue.timestamp));
  Serial.flush();
  /*
  for(int i = sizeof(IUE_t)-1; i != 0; i--)
  {
    Serial.print("<");
    Serial.print((int8_t) iue_ptr[i], HEX);
    Serial.print(">");
  }
  */
}
#endif

#endif
