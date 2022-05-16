#include "wearableDataQueue.h"

wearableDataQueue::wearableDataQueue()
{
}

void wearableDataQueue::begin()
{
  flash = Adafruit_SPIFlash(&flashTransport);
  while(!flash.begin());
  while(!fs.begin(&flash));
  initializeHeadTail();
}

void wearableDataQueue::enqueue(wearable_data_t wd)
{
  char fileName[DATA_FILENAME_MAX_LENGTH];
  i_to_cstr(tail++, fileName, DATA_FILENAME_MAX_LENGTH);
#ifdef PIN_SERIAL_ON
  Serial.print(F("enqueuing "));
  Serial.print(fileName);
  Serial.print(F(" with "));
  printWearableData(wd);
  Serial.println("");
#endif
  if(fs.exists(fileName))
    fs.remove(fileName);
  File wearableDataFile = fs.open(fileName, FILE_WRITE);
  wearableDataFile.write(&wd, sizeof(wearable_data_t));
  wearableDataFile.close();
  updateTailFile();
}

wearable_data_t wearableDataQueue::dequeue()
{
  wearable_data_t wd = {0, 0, 0, 0, 0};
  char fileName[DATA_FILENAME_MAX_LENGTH];
  i_to_cstr(head++, fileName, DATA_FILENAME_MAX_LENGTH);
  if(fs.exists(fileName))
  {
    File wearableDataFile = fs.open(fileName, FILE_READ);
#ifdef PIN_SERIAL_ON
    Serial.print(F("opened file with size "));
    Serial.println(wearableDataFile.size());
#endif
    wearableDataFile.read(&wd, sizeof(wearable_data_t));
    wearableDataFile.close();
    fs.remove(fileName);
  }
#ifdef PIN_SERIAL_ON
  Serial.print(F("dequeued "));
  printWearableData(wd);
  Serial.print(F(" from "));
  Serial.println(fileName);
#endif
  if(head == tail)
  {
    head = 0;
    tail = 0;
  }
  updateHeadFile();
  return wd;
}

uint16_t wearableDataQueue::size()
{
  return tail - head;
}

//converts len-1 digits (lsd to msd) of an integer n to a c-string s
void wearableDataQueue::i_to_cstr(uint64_t n, char* s, const int len)
{
  //get number of digits in n
  int digits = 1;
  for(uint64_t i = n/10; i != 0; i /= 10)
    digits++;

  //determine whether to use digits or len as end of string
  bool useDigits = false;
  if(digits < len)
    useDigits = true;

  //place null terminator
  s[useDigits ? digits : len-1] = 0;
    
  //make string
  int mod = 10;
  //for(int i = 0; i < digits-1; i++) mod *= 10;
  for(int i = (useDigits ? digits-1 : len-2); i >= 0; i--)
  {
    s[i] = (n % mod) + 48; //48 is ascii for '0'
    n /= 10;
    //mod /= 10;
  }
#ifdef PIN_SERIAL_ON
  Serial.print(F("i_to_cstr converted "));
  Serial.print(F("<"));
  Serial.print(s);
  Serial.println(F(">"));
  Serial.flush();
#endif
}

void wearableDataQueue::updateHeadFile()
{
  #ifdef PIN_SERIAL_ON
  Serial.print(F("updating head file to "));
  Serial.println(head);
#endif
  if(fs.exists(HEAD_FILENAME))
    fs.remove(HEAD_FILENAME);
  File headFile = fs.open(HEAD_FILENAME, FILE_WRITE);
  headFile.print(head);
  headFile.close();
}

uint16_t wearableDataQueue::getHeadFromFile()
{
  if(!fs.exists(HEAD_FILENAME))
    return 0;
  File headFile = fs.open(HEAD_FILENAME, FILE_READ);
  uint16_t _head = headFile.parseInt();
  headFile.close();
#ifdef PIN_SERIAL_ON
  Serial.print(F("retreiving from head file: "));
  Serial.println(_head);
#endif  
  return _head;
}

void wearableDataQueue::updateTailFile()
{
#ifdef PIN_SERIAL_ON
  Serial.print(F("updating tail file to "));
  Serial.println(tail);
#endif
  if(fs.exists(TAIL_FILENAME))
    fs.remove(TAIL_FILENAME);
  File tailFile = fs.open(TAIL_FILENAME, FILE_WRITE);
  tailFile.print(tail);
  tailFile.close();
}

uint16_t wearableDataQueue::getTailFromFile()
{
  if(!fs.exists(TAIL_FILENAME))
    return 0;
  File tailFile = fs.open(TAIL_FILENAME, FILE_READ);
  uint16_t _tail = tailFile.parseInt();
  tailFile.close();
#ifdef PIN_SERIAL_ON
  Serial.print(F("retreiving from tail file: "));
  Serial.println(_tail);
#endif  
  return _tail;
}

void wearableDataQueue::initializeHeadTail()
{
  head = getHeadFromFile();
  tail = getTailFromFile();
  
  if(head == 0 && tail ==0)
    return;

  if(head < tail || head == tail)
  {
    //check for files above and below head and tail
    char fileName[DATA_FILENAME_MAX_LENGTH];
    i_to_cstr(head-1, fileName, DATA_FILENAME_MAX_LENGTH);
    if(fs.exists(fileName))
    {
      head -= 1;
      updateHeadFile();
    }

    i_to_cstr(tail+1, fileName, DATA_FILENAME_MAX_LENGTH);
    if(fs.exists(fileName))
    {
      tail += 1;
      updateHeadFile();
    }
  }
  
  if(head > tail)
  {
    //check for files up to tail to see where head should be
    for(uint16_t i = 0; i != tail; i++)
    {
      char fileName[DATA_FILENAME_MAX_LENGTH];
      i_to_cstr(i, fileName, DATA_FILENAME_MAX_LENGTH);
      if(fs.exists(fileName))
      {
        head = i;
        updateHeadFile();
        break;
      }
    }
    if(head > tail)
    {
      //no files up to tail, so reset
      head = 0;
      tail = 0;
      updateHeadFile();
      updateTailFile();
    }
  }
}

bool wearableDataQueue::empty()
{
  return (head == tail);
}
