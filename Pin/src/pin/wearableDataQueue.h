#ifndef WEARABLE_DATA_QUEUE_H
#define WEARABLE_DATA_QUEUE_H

#include "Arduino.h"
#include "wearable_data_t.h"
#include <SdFat.h>
#include <SPI.h>
#include <Adafruit_SPIFlash.h>

#include "pinDebug.h"

//name of files that will hold queue head and tail in case of power loss
#define HEAD_FILENAME "head"
#define TAIL_FILENAME "tail"

//files are named as 16-bit integers, 5 is the most digits a 16-bit int will have. 5 digits + null terminator = 6
#define DATA_FILENAME_MAX_LENGTH 6

class wearableDataQueue
{
  public:
  wearableDataQueue();
  void begin();
  void enqueue(wearable_data_t);
  wearable_data_t dequeue();
  uint16_t size();
  bool empty();
  
  private:
  FatFileSystem fs;
  Adafruit_FlashTransport_QSPI flashTransport;
  Adafruit_SPIFlash flash;
  
  uint16_t head;
  uint16_t tail;

  void i_to_cstr(uint64_t n, char* s, const int len);
  void updateHeadFile();
  void updateTailFile();
  uint16_t getHeadFromFile();
  uint16_t getTailFromFile();
  void initializeHeadTail();
  
};

#endif
