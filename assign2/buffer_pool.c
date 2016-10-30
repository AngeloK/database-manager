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
  
  
  //Init hash, it's important when tesing in server end.
  int i;
  for (i = 0; i < 65535; i++) {
    bs->mapping[i] = NULL;
  }
  return bs;
}

Queue *createQueue(int capacity) {
  
  printf("queue size %d\n", capacity);
  Queue* queue = (Queue *)malloc( sizeof( Queue ) );

  // The queue is empty
  queue->lru_lastUsed = 0;
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
    temp->fix_count = 1;
    temp->lastUsed = 0;
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
  // add->next = NULL;
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

// int replaceByLRU (BM_BufferPool *bm, Page_Frame *remove, Page_Frame *add) {
// 	
// 	// Retrieve pool queue and capacity from BufferPool
// 	Buffer_Storage *bs = (Buffer_Storage*) bm->mgmtData;
// 	Queue *pool = bs->pool;
// 
// 	// Start with the front of the pool
// 	Page_Frame *lowest = NULL;
// 	Page_Frame *current = pool->front;
// 	
// 	// Search through the queue for lowest last used time
// 	while (current != NULL) {
// 		
// 		if (current->fix_count > 0) { // No point proceeding. Cannot Replace
//       current = current->next;
// 			continue;
// 		} else if (lowest == NULL) { // Lowest has no prior assignment
// 			lowest = current;
// 		} else {
// 					
// 			if (lowest->lastUsed < current->lastUsed) {
// 				lowest = current;
// 			}
// 			current = current->next;
// 		}
// 	}
// 	
// 	if (lowest == NULL) {
// 		return -1;
// 	}

	// Remove the lowest found time
// 	remove = lowest;
// 
// 	// Put new buffer in its place
// 	if (lowest->prev == NULL) {
// 		pool->front = add;
// 	} else {
// 		lowest->prev->next = add;
// 	}
// 	add->prev = lowest->prev;
// 
// 	if (lowest->next == NULL) {
// 		pool->rear = add;
// 	} else {
// 		lowest->next->prev = add;
// 	}
// 	add->next = lowest->next;
// 
//   printf("remove in LRU is %d\n", remove->pageHandle->pageNum);
// 	return remove->pageHandle->pageNum;
// }


int Replacement(Queue *queue, Page_Frame *removed, Page_Frame *added){
    // If all frames are full, remove the page at the rear
    printf("queue count is %d\n", queue->count);
    if (queue->count == queue->q_capacity){
      
      int replaced;
      replaced = deQueue(queue);
      enQueue(queue, added);
      return replaced;
    }
    
    else {
      printQueueElement(queue);
      enQueue(queue, added);
      printQueueElement(queue);
    }
    return -1;
}


int enQueue(Queue *queue, Page_Frame *added) {

  printf("enQueue %d\n", added->pageHandle->pageNum);
  // Page_Frame *temp = added;
  
  if (queue->count == 0) {
    printf("queue is empty\n");
    queue->rear = queue->front = added;
  }
  else {
    queue->rear->next = added;
    added->prev = queue->rear;
    queue->rear = added;
    printf("queue front is %p\n",queue->front);
  }
  queue->count++;
  printf("after adding new node, queue count is %d\n", queue->count);
  return 1;
}

int printQueueElement(Queue *queue) {
  printf("===pool====\n");
  Page_Frame *f = queue->front;
  int i;
  while(f) {
    printf("addresss is %p, pageNum is %d is_dirty=%d\n", f, f->pageHandle->pageNum, f->is_dirty);
    f = f->next;
  }
  printf("===pool end====\n");
  return 1;
} 

int checkBufferBusy(Queue *queue) {
  Page_Frame *temp = queue->front;
  int is_busy = 1; //1 true, 0 false, default is 1;
  
  while(temp) {
    if (temp->fix_count == 0) {
      printf("temp check is %d, fix_count is %d\n", temp->pageHandle->pageNum, temp->fix_count);
      is_busy = 0;
      break;
    }
    else
      temp = temp->next;
  }
  return is_busy;
}

int deQueue( Queue *queue )
{
  printf("queue count is %d\n", queue->count);
    int removed;
    // queue is empty.
    if(queue->count == 0)
        return -1;
    
    // if the none of elements in queue has fix_count=0, 
    // then buffer pool is busy, we can not replace elements.
    if (checkBufferBusy(queue) == 1) {
      printf("buffer is busy\n");
      return RC_BUFFER_BUSY;
    }
    else {
    // If this is the only node in list, then change front
      if (queue->count == 1) {
          removed = queue->rear->pageHandle->pageNum;
          //empty a queue.
          queue->front = NULL;
          queue->rear = NULL;
          return removed;
      }
        
      // Page_Frame *temp = queue->front;
      removed = queue->front->pageHandle->pageNum;
      
      // printf("temp->is_dirty= %d\n", temp->is_dirty);
      queue->front = queue->front->next;
      queue->count--;
      printf("removed in deQueue is %d\n", removed);
      
      // free(temp);
      return removed;
  }

}
