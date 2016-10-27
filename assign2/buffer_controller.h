#ifndef BUFFER_CONTROLLER_H
#define BUFFER_CONTROLLER_H

#include <stdbool.h>
#include "buffer_mgr.h"

typedef long int Timestamp;


// Definition of the page frame.
typedef struct Page_Frame {
  int fix_count; // total number of times the page is pinned.
  bool is_dirty; // the page is dirty of not.
  int pageNum;   //  page number of the page in page file.
  Timestamp last_access; // the last access timestamp.
  int clock_bit; // clock bit is used for clock replacement strategy.
  char *data;    // the pointer to the page content stored in memeory.
} Page_Frame, Pool;


// data structure used for storing page frames.
typedef struct Buffer_Storage {
  int size;  // size of buffer pool.
  int count; // the number of page frames which are occupied.
  int curPos;     // current position, it's used for FIFO.
  int numReadIO; // the number of pages that have been read from disk since a buffer pool has been initialized
  int numWriteIO; // the number of pages that have been written to disk since a buffer pool has been initialized
  Pool *pool;
} Buffer_Storage;

//macros.
#define isBufferStorageFull(bs) (bs->count == bs->size)


Buffer_Storage *initBufferStorage(int size);
void destroyBufferStorage(Buffer_Storage *bs);
void updatePageFrame(Page_Frame *pf, BM_PageHandle *const page);

#endif
