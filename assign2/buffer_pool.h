#ifndef BUFFER_POOL_H
#define BUFFER_POOL_H

#include <stdbool.h>
#include "buffer_mgr.h"


typedef struct Page_Frame {
  int fix_coutn;
  bool is_dirty;
  BM_PageHandle *pageHandle;
  
  struct Page_Frame *prev;
  struct Page_Frame *next;
} Page_Frame;

typedef struct Queue {
  Page_Frame *front;
  Page_Frame *rear;
  int count;
  int capacity;
  int readIO;
  int writeIO;
} Queue;

typedef struct Hash
{
  int capacity; // how many pages can be there
  Page_Frame **array; // an array of queue nodes
} Hash;


typedef struct Buffer_Storage {
  Hash *mapping;
  Queue *pool;
}

#endif


Buffer_Storage *initBufferStorage(char *pageFileName, int capacity) {
Queue *createQueue(int capacity);
Hash *createHash(int totalNumPages);