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


RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, 
		  const int numPages, ReplacementStrategy strategy, 
		  void *stratData){
        
    bm->pageFile = (char *)pageFileName;
    bm->numPages = numPages;
    bm->strategy = strategy;
		printf("numPages is %d\n", numPages);
    bm->mgmtData = (Buffer_Storage *)initBufferStorage(pageFileName, numPages);
    
  return RC_OK;
}

RC shutdownBufferPool(BM_BufferPool *const bm) {
	SM_FileHandle fHandle;
  Buffer_Storage *bs = (Buffer_Storage*)bm->mgmtData;
  Queue *q = bs -> pool;
  CHECK(openPageFile(bm->pageFile, &fHandle))
  Page_Frame *temp = q->front; 
  while(temp!=NULL){
	if(temp-> fix_count>0){
		return RC_CANNOT_SHUTDOWN;
	}
	if(temp->is_dirty == TRUE ){
		CHECK(writeBlock(temp-> pageHandle->pageNum, &fHandle, temp-> pageHandle->data));
		temp->is_dirty = FALSE;
		q->writeIO++;
		
	}
	temp = temp->next;
  }
  CHECK(closePageFile( &fHandle))
  free(q->front);
  free(q->rear);
  free(q);
  // free(bs->mapping);
  free(bs);
  q = NULL;
  bs = NULL;

  return RC_OK;
}

RC forceFlushPool(BM_BufferPool *const bm) {
	// SM_FileHandle fHandle;   
	// Buffer_Storage *bs = (Buffer_Storage*)bm->mgmtData;
	// Queue *q = bs -> pool;
	// CHECK(openPageFile(bm->pageFile, &fHandle))
	// Page_Frame *temp = q->front; 
	// while(temp!=NULL){
	// 	if(temp->is_dirty == TRUE && temp-> fix_count == 0 ){
	// 		CHECK(writeBlock(temp-> pageHandle->pageNum, &fHandle, temp-> pageHandle->data));
	// 		temp->is_dirty = FALSE;
	// 		q->writeIO++;
	// 		
	// 	}
	// 	temp = temp->next;
	// }
	// CHECK(closePageFile( &fHandle));
	return RC_OK;
}

RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, 
	    const PageNumber pageNum) {
	// pageNum is found in mapping table.
	Buffer_Storage *bs = (Buffer_Storage *)bm->mgmtData;
	if (bs->mapping[pageNum]) {
		page->pageNum = pageNum;
		page->data = (bs->mapping[pageNum])->pageHandle->data;
		return RC_OK;
	}
	Queue *pool = bs->pool;
	
	// BM_PageHandle *loadedPage = loadFromPageFile(bm->pageFile, pageNum);
	
	// pageNum is not found
	Page_Frame *removed;
	BM_PageHandle *p = (BM_PageHandle *)page;
	
	p->pageNum = pageNum;
	Page_Frame *added  = newPageFrame(pageNum, page); 
	
	
	
	if (isPoolFull(bm)) {
		// printf("it's full\n");
		// if (bm->strategy == RS_FIFO) {
		// 	replaceByFIFO(bm, removed, added);
		// }
		// if (bm->strategy == RS_LFU) {
		// 	// LRU;
		// }
		return 0;
	}
	else {
		
		
		if (pool->count == 0) {
			// pool is empty;
			
			pool->front = added;
			pool->rear = added;
			pool->count++;
			
		}
		else {
			pool->front->prev = added;
			pool->front = added;
			pool->count++;
			// bs->pool->rear->next = added;
			// added->prev = bs->pool->rear;
			// bs->pool->rear = added;
			// bs->pool->q_capacity++;
		}
	}
	
	// added new mapping.
	// bs->mapping[pageNum] = added;
	
	// if (removed->is_dirty) {
	// 	writeToDisk(bm, removed);
		// write to disk.
	// }
	
	//assinge new to pageHandle;
	// page->pageNum = added->pageHandle->pageNum;
	// page->data = added->pageHandle->data;
	return RC_OK;
}


// bool *getDirtyFlags (BM_BufferPool *const bm)
// {
//   int index = 0;
//   Buffer_Storage *md = (Buffer_Storage*)bm->mgmtData;
//   Queue *q = md -> pool;
//   bool *count = (bool *)malloc(sizeof(bool)*bm->numPages);
//   Page_Frame *temp = q->front;
//   while(temp!=NULL){
// 	count[index++]=temp->is_dirty;
// 	temp = temp->next;
//   }
// 
//  return count;
// }
// 
// int *getFixCounts (BM_BufferPool *const bm)
// {
//   int index = 0;
//   Buffer_Storage *md = (Buffer_Storage*)bm->mgmtData;
//   Queue *q = md -> pool;
//   int *count = (int *)malloc(sizeof(int)*bm->numPages);
//   Page_Frame *temp = q->front;
//     while(temp!=NULL){
// 	count[index++]=temp-> fix_count;
// 	temp = temp->next;
//   }
//   return count;
// }
// 
// int getNumReadIO (BM_BufferPool *const bm)
// {
//   Buffer_Storage *md = (Buffer_Storage*)bm->mgmtData;
//   Queue *q = md -> pool;
//   return q-> readIO;
// }
// 
// int getNumWriteIO (BM_BufferPool *const bm)
// {
//   Buffer_Storage *md = (Buffer_Storage*)bm->mgmtData;
//   Queue *q = md -> pool;
//   return q-> writeIO;
// }
