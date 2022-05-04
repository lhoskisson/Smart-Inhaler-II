#include "iueQueue.h"

iueQueue:iueQueue()
{
  queueSize = 0;
}

void iueQueue::enqueue(IUE_t iue)
{
  char[IUE_FILENAME_MAX_LENGTH] fileName;
  i_to_cstr(queueSize++, fileName, IUE_FILENAME_MAX_LENGTH);
  File iueFile = fs.open(fileName, FILE_WRITE);
  iueFile.print(iue.timestamp);
  iueFile.close();
  updateQueueSizeFile();
}

IUE_t iueQueue::dequeue()
{
  IUE_t iue;
  iueQueueFile.open(IUE_QUEUE_FILE_NAME, FILE_WRITE);
  long retVal = 0;
  if(iueQueueFile.available())
  {
    retVal = iueQueueFile.parseInt();
    
  }
  
#ifdef INHALER_SERIAL_ON
  if(sizeof(long) != sizeof(IUE_t)) Serial.println("IUE Size Mismatch from file");
#endif

  iue.timestamp = retVal;
  return iue;
}

int16_t iueQueue::getQueueSize()
{
  return _queueSize();
}

//converts len-1 digits (lsd to msd) of an integer n to a c-string s
void iueQueue::i_to_cstr(int16_t n, char* s, const int len)
{
  //get number of digits in n
  int digits = 1;
  for(int i = n/10; i != 0; i /= 10)
    digits++;

  //determine whether to use digits or len as end of string
  bool useDigits = false;
  if(digits < len)
    useDigits = true;

  //place null terminator
  s[useDigits ? digits : len-1] = 0;
    
  //make string
  int mod = 10;
  for(int i = 0; i < digits-1; i++) mod *= 10;
  for(int i = 0; i < (useDigits ? digits : len-1); i++)
  {
    s[i] = (n % mod) + 48; //48 is ascii for '0'
    mod /= 10;
  }
}

void iueQueue::updateQueueSizeFile()
{
}
