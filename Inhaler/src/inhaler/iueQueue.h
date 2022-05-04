#ifndef IUE_QUEUE_H
#define IUE_QUEUE_H

#include "IUE_t.h"
#include <SdFat.h>

#define IUE_QUEUE_FILE_NAME "iueQueue.csv"
#define IUE_FILENAME_MAX_LENGTH (6) //files are named as 16-bit integers, 5 is the most digits a 16-bit int will have. 5 digits + null terminator = 6


class iueQueue
{
  public:
  iueQueue();
  void enqueue(IUE_t);
  IUE_t dequeue();
  int getQueueSize();
  
  private:
  FatFileSystem fs;
  
  //file where queue size is stored in case of device shutdown during transfer of queue to app
  File queueSize; 

  
  int16_t _queueSize;

  void i_to_cstr(int16_t n, char* s, const int len);

  void updateQueueSizeFile();
  
};

#endif
