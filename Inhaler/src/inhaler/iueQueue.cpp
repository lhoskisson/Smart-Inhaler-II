#include "iueQueue.h"

iueQueue:iueQueue()
{
  initializeHeadTail();
}

void iueQueue::enqueue(IUE_t iue)
{
  char[IUE_FILENAME_MAX_LENGTH] fileName;
  i_to_cstr(tail++, fileName, IUE_FILENAME_MAX_LENGTH);
#ifdef INHALER_SERIAL_ON
  Serial.print("enqueuing ");
  Serial.print(fileName);
  Serial.print(" with ");
  Serial.println(iue.timestamp);
#endif
  noInterrupts();
  if(fs.exists(fileName))
    fs.remove(fileName);
  File iueFile = fs.open(fileName, FILE_WRITE);
  iueFile.print(iue.timestamp);
  iueFile.close();
  interrupts();
  updateQueueTailFile();
}

IUE_t iueQueue::dequeue()
{
  IUE_t iue;
  iue.timestamp = 0;
  char[IUE_FILENAME_MAX_LENGTH] fileName;
  i_to_cstr(head++, fileName, IUE_FILENAME_MAX_LENGTH);
  if(fs.exists(fileName))
  {
    noInterrupts();
    File iueFile = fs.open(fileName, FILE_READ);
    iue.timestamp = iueFile.parseInt();
    iueFile.close();
    fs.remove(fileName);
    interrupts();
  }
#ifdef INHALER_SERIAL_ON
  if(sizeof(long) != sizeof(IUE_t)) Serial.println("IUE Size Mismatch from file");
  Serial.print("dequeued ");
  Serial.print(iue.timestamp);
  Serial.print(" from ");
  Serial.println(fileName);
#endif
  if(head == tail)
  {
    head = 0;
    tail = 0;
  }
  return iue;
}

int16_t iueQueue::getQueueSize()
{
  return tail - head;
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
#ifdef INHALER_SERIAL_ON
  Serial.print("i_to_cstr converted int ");
  Serial.print(n);
  Serial.print(" to cstr ");
  Serial.println(s);
#endif
}

void iueQueue::updateHeadFile()
{
  #ifdef INHALER_SERIAL_ON
  Serial.print("updating head file to ");
  Serial.println(head);
#endif
  noInterrupts();
  if(fs.exists(HEAD_FILENAME))
    fs.remove(HEAD_FILENAME);
  File headFile = fs.open(HEAD_FILENAME, FILE_WRITE);
  headFile.print(head);
  headFile.close();
  interrupts();
}

int16_t iueQueue::getHeadFromFile()
{
  noInterrupts();
  if(!fs.exists(HEAD_FILENAME))
    return 0;
  File headFile = fs.open(HEAD_FILENAME, FILE_READ);
  int16_t _head = queueSizeFile.parseInt();
  headFile.close();
  interrupts();
#ifdef INHALER_SERIAL_ON
  Serial.print("retreiving from head file: ");
  Serial.println(_head);
#endif  
  return _head;
}

void iueQueue::updateTailFile()
{
#ifdef INHALER_SERIAL_ON
  Serial.print("updating tail file to ");
  Serial.println(tail);
#endif
  noInterrupts();
  if(fs.exists(TAIL_FILENAME))
    fs.remove(TAIL_FILENAME);
  File tailFile = fs.open(TAIL_FILENAME, FILE_WRITE);
  tailFile.print(tail);
  tailFile.close();
  interrupts();
}

int16_t iueQueue::getTailFromFile()
{
  noInterrupts();
  if(!fs.exists(TAIL_FILENAME))
    return 0;
  File tailFile = fs.open(TAIL_FILENAME, FILE_READ);
  int16_t _tail = queueSizeFile.parseInt();
  tailFile.close();
  interrupts();
#ifdef INHALER_SERIAL_ON
  Serial.print("retreiving from tail file: ");
  Serial.println(_tail);
#endif  
  return _tail;
}

void iueQueue::initializeHeadTail()
{
  head = getHeadFromFile();
  tail = getTailFromFile();

  if(head == 0 && tail ==0)
    return;

  if(head < tail)
  {
    //check for files above and below head and tail
  }
  
  if(head == tail)
  {
    //check for files above and below
  }
  
  if(head > tail)
  {
    //check for files up to tail to see where head should be  
  }
}
