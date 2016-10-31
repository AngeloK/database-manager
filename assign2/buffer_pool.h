#ifndef BUFFER_POOL_H
#define BUFFER_POOL_H

#include <stdbool.h>
#include "buffer_mgr.h"


typedef struct Page_Frame {
  int fix_count;
  bool is_dirty;
  BM_PageHandle *pageHandle;
  int lastUsed;
  struct Page_Frame *prev;
  struct Page_Frame *next;
  int index;
} Page_Frame;

typedef struct Queue {
  Page_Frame *front;
  Page_Frame *rear;
  int count;
  int q_capacity;
  int readIO;
  int writeIO;
  int lru_lastUsed;
} Queue;


// Depracted
typedef struct Hash
{
  int capacity; // how many pages can be there
  Page_Frame **array; // an array of queue nodes
} Hash;


typedef struct Buffer_Storage {
	Page_Frame *mapping[65535];
	Queue *pool;
} Buffer_Storage;


Buffer_Storage *initBufferStorage(char *pageFileName, int capacity);
Queue *createQueue(int capacity);
Hash *createHash(int totalNumPages);
Page_Frame* newPageFrame(int pageNum, BM_PageHandle *page);

int replaceByFIFO (BM_BufferPool *bm, Page_Frame *remove, Page_Frame* add);
int replaceByLRU (BM_BufferPool *bm, Page_Frame *remove, Page_Frame* add);
int isPoolFull(BM_BufferPool *bm);
int writeToDisk(BM_BufferPool *bm, BM_PageHandle *page);
int ReplacementFIFO(Queue *queue, Page_Frame **mapping, Page_Frame *removed, Page_Frame *added);
int ReplacementLRU(Queue *queue, Page_Frame **mapping, Page_Frame *removed, Page_Frame *added);

#endif
