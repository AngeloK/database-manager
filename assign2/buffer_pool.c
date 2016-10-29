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
  Queue *pool = createQueue(capacity);
  Buffer_Storage *bs;
  bs = (Buffer_Storage *)malloc(sizeof(Buffer_Storage));
  bs->pool = pool;
  return bs;
}

Queue *createQueue(int capacity) {
  
  printf("queue size %d\n", capacity);
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
// 
// Page_Frame *loadFromPageFile(char *pageFileName, PageNumber pageNum) {
//     SM_FileHandle fh;
//     openPageFile(pageFileName, &fh);
//     SM_PageHandle ph;
//     ph = (SM_PageHandle) malloc(PAGE_SIZE);
//     
//     if (fh.totalNumPages < pageNum) {
//       ensureCapacity(pageNum+1, &fh);
//     }
//     readBlock(pageNum, &fh, ph);
// 
//     BM_PageHandle *pageHandle = MAKE_PAGE_HANDLE();
//     
//     pageHandle->pageNum = pageNum;
//     pageHandle->data = ph;
//     
//     closePageFile(&fh);
//     return pageHandle;
// }

int isPoolFull(BM_BufferPool *bm) {
  Buffer_Storage *bs = (Buffer_Storage*)bm->mgmtData;
  Queue *q = bs->pool;
  if(q->count==q->q_capacity){
    printf("in isPoolFull %d, %d\n", q->q_capacity, q->count);
    return 1;
  }
  else{
    return 0;
  }
}
RC writeToDisk(BM_BufferPool *bm, BM_PageHandle *page) {
  printf("data written is %s\n", page->data);
  SM_FileHandle fh;
  openPageFile(bm->pageFile, &fh);
  SM_PageHandle ph;
  ph = (SM_PageHandle) malloc(PAGE_SIZE);
  if (fh.totalNumPages < page->pageNum) {
    ensureCapacity(page->pageNum+1, &fh);
  }
  ph = page->data;
  writeBlock(page->pageNum, &fh, ph);
  closePageFile(&fh);
  free(ph);
}

int replaceByFIFO (BM_BufferPool *bm, Page_Frame *remove, Page_Frame *add) {
  Buffer_Storage *bs = (Buffer_Storage *)bm->mgmtData;
  
	// Retrieve pool queue and capacity from BufferPool
	Queue *pool = bs->pool;
  
	int size = pool->q_capacity;
	
	// Add new page to the end of the pool queue
	if (pool->rear != NULL) {
		pool->rear->next = add;
	}
	add->prev = pool->rear;
	pool->rear = add;
  
	int i = 0;
  
	// Initialize head to pool queue head	
	remove = pool->front;
	while (remove != NULL) {	
		// Find the first in queue to have 0 fix_count
		if (remove->fix_count == 0) {
			if (remove->prev == NULL) {
				pool->front = remove->next;
			} else {
				remove->prev->next = remove->next;
			}

			if (remove->next == NULL) {
				pool->rear = remove->prev;
			} else {
				remove->next->prev = remove->prev;
			}
			break;
		} 
		remove = remove->next;
	}
  
	if (remove == NULL) {
		return -1;
	}
  
  printf("remove in FIFO is %d\n", remove->pageHandle->pageNum);
  
	return remove->pageHandle->pageNum;
}