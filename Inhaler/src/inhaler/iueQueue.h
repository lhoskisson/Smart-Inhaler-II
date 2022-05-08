#ifndef IUE_QUEUE_H
#define IUE_QUEUE_H

#include "Arduino.h"
#include "IUE_t.h"
#include <SdFat.h>

#include "inhalerDebug.h"

//name of files that will hold queue head and tail in case of power loss
#define HEAD_FILENAME "head"
#define TAIL_FILENAME "tail"

//files are named as 16-bit integers, 5 is the most digits a 16-bit int will have. 5 digits + null terminator = 6
#define IUE_FILENAME_MAX_LENGTH 6

//20 digits possible for 64 bit decimal number
#define IUE_MAX_LENGTH 20

class iueQueue
{
  public:
  iueQueue();
  void enqueue(IUE_t);
  IUE_t dequeue();
  int16_t size();
  bool empty();
  
  private:
  FatFileSystem fs;
  
  int16_t head;
  
  int16_t tail;

  void i_to_cstr(int64_t n, char* s, const int len);

  void updateHeadFile();
  void updateTailFile();
  int16_t getHeadFromFile();
  int16_t getTailFromFile();
  void initializeHeadTail();
  
};

#endif
