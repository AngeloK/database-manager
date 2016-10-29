#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include "storage_mgr.h"
#include "dberror.h"
#include "test_helper.h"
#include "buffer_pool.h"



Buffer_Storage *initBufferStorage(char *pageFileName, int capacity) {
  SM_FileHandle *fh;
  openPageFile(pageFileName, &fh);
  int totalNumPages = fh->totalNumPages;
  closePageFile(&fh);
  // Hash *mapping = createHash(totalNumPages);
  Queue *pool = createQueue(capacity);
  Buffer_Storage *bs;
  // bs->mapping = mapping;
  bs->pool = pool;
  return bs;
  
}

Queue *createQueue(int capacity) {
  Queue* queue = (Queue *)malloc( sizeof( Queue ) );

  // The queue is empty
  queue->count = 0;
  queue->front = queue->rear = NULL;

  // Number of frames that can be stored in memory
  queue->q_capacity = capacity;

  return queue;
}

// Hash* createHash( int totalNumPages);
// {
//     // Allocate memory for hash
//     Hash* hash = (Hash *) malloc( sizeof( Hash ) );
//     hash->capacity = capacity;
//  
//     // Create an array of pointers for refering queue nodes
//     hash->array = (Page_Frame **) malloc( hash->capacity * sizeof( Page_Frame* ) );
//  
//     // Initialize all hash entries as empty
//     int i;
//     for( i = 0; i < hash->capacity; ++i )
//         hash->array[i] = NULL;
//  
//     return hash;
// }

Page_Frame* newPageFrame( int pageNum , BM_PageHandle *page)
{
    Page_Frame* temp = (Page_Frame *)malloc( sizeof( Page_Frame ) );
    // temp->pageNumber = pageNum;
    temp->pageHandle = page;
    temp->prev = temp->next = NULL;
    temp->is_dirty = FALSE;
    temp->fix_count = 0;
    return temp;
}

Page_Frame *loadFromPageFile(char *pageFileName, PageNumber pageNum) {
    SM_FileHandle fh;
    openPageFile(pageFileName, &fh);
    SM_PageHandle ph;
    ph = (SM_PageHandle) malloc(PAGE_SIZE);
    
    if (fh.totalNumPages < pageNum) {
      ensureCapacity(pageNum+1, &fh);
    }
    readBlock(pageNum, &fh, ph);

    BM_PageHandle *pageHandle = MAKE_PAGE_HANDLE();
    
    pageHandle->pageNum = pageNum;
    pageHandle->data = ph;
    return pageHandle;
}

int isPoolFull(BM_BufferPool *bm) {
  Buffer_Storage *bs = (Buffer_Storage*)bm->mgmtData;
  Queue *q = bs->pool;
  if(q->count==q->q_capacity){
    return 1;
  }
  else{
    return 0;
  }
}
RC writeToDisk(BM_PageHandle *page);