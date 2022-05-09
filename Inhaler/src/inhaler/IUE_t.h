#ifndef IUE_T_H
#define IUE_T_H

#include "Arduino.h"

#include "inhalerDebug.h"

typedef struct{
  uint64_t timestamp;      // 8 bytes
} IUE_t;

#ifdef INHALER_SERIAL_ON
static void printIUE(IUE_t iue)
{
 int8_t* iue_ptr = (int8_t*) &iue;
  Serial.print("IUE: ");
  for(int i = sizeof(IUE_t)-1; i != 0; i--)
  {
    Serial.print("<");
    Serial.print((int8_t) iue_ptr[i], HEX);
    Serial.print(">");
  }
}
#endif

#endif
