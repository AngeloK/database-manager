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
	if(temp->is_dirty == true ){
		printf("temp is %s\n", temp->pageHandle->data);
		CHECK(writeBlock(temp-> pageHandle->pageNum, &fHandle, temp-> pageHandle->data));
		temp->is_dirty = FALSE;
		q->writeIO++;
		
	}
	else {
		printf("left is not dirty\n");
		printf("temp is %s\n", temp->pageHandle->data);
	}
	temp = temp->next;
  }
  CHECK(closePageFile(&fHandle))
  // free(q->front);
  // free(q->rear);
  // free(q);
	
	//free mapping.
	int i;
	printf("free mapping\n");
	for (i = 0; i < 10000; i++) {
		bs->mapping[i] = NULL;
	}
	
  free(bs);
  q = NULL;
  bs = NULL;
	printf("shutdown\n");

  return RC_OK;
}

RC forceFlushPool(BM_BufferPool *const bm) {
	SM_FileHandle fHandle;   
	Buffer_Storage *bs = (Buffer_Storage*)bm->mgmtData;
	Queue *q = bs -> pool;
	CHECK(openPageFile(bm->pageFile, &fHandle))
	Page_Frame *temp = q->front; 
	while(temp!=NULL){
		// printf("temp->%d\n", temp->pageHandle->pageNum );
		if(temp->is_dirty == TRUE && temp-> fix_count == 0 ){
			CHECK(writeBlock(temp-> pageHandle->pageNum, &fHandle, temp-> pageHandle->data));
			temp->is_dirty = FALSE;
			q->writeIO++;
			
		}
		temp = temp->next;
	}
	CHECK(closePageFile( &fHandle));
	return RC_OK;
}

RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page) {
	SM_FileHandle fHandle;   
	Buffer_Storage *bs = (Buffer_Storage*)bm->mgmtData;
	Queue *q = bs -> pool;
	CHECK(openPageFile(bm->pageFile, &fHandle));

	ensureCapacity(page->pageNum, &fHandle);
	writeBlock(page->pageNum, &fHandle, page->data);
	q->writeIO++;
	CHECK(closePageFile( &fHandle));
	return RC_OK;
}


RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, 
	    const PageNumber pageNum) {
	// pageNum is found in mapping table.
	
	printf("pinPage is %d\n", pageNum);
	Buffer_Storage *bs = (Buffer_Storage *)bm->mgmtData;
	
  SM_FileHandle fh;
  SM_PageHandle ph;
  ph = (SM_PageHandle) malloc(PAGE_SIZE);
	BM_PageHandle *p = MAKE_PAGE_HANDLE();
	int replaced;
	Page_Frame *removed;
	Page_Frame *added  = newPageFrame(pageNum, p); 
	Queue *pool = bs->pool;
	
	
	printf("===hash table====\n");
	// if (pool->front) {
	// 	printf("queue front is %d\n", pool->front->pageHandle->pageNum);
	// 	if (pool->front->next){
	// 		printf("queue \n", );
	// 	}
	// }
	if (pool->rear)
		printf("queue rear is %d\n", pool->rear->pageHandle->pageNum);
	// for (size_t i = 0; i < 20; i++) {
	// 	if (bs->mapping[i] != NULL) {
	// 		printf("mapped pageNum is %d\n", i);
	// 	}
	// 	/* code */
	// }
	printf("=====hash table end====\n");
	
	// read from mapping.
	if (bs->mapping[pageNum]) {
		// page->pageNum = pageNum;
		// printf("bs->mapping->pageHandle->data=%s\n", bs->mapping[pageNum]->pageHandle->data);
		// printf("page->data %p\n", page->data);
		// page->data = (bs->mapping[pageNum])->pageHandle->data;
			
		printf("found in mapping %d\n", pageNum);
		page->data = (bs->mapping[pageNum])->pageHandle->data;
		// page->data = pageNum;
		
		return RC_OK;
	}
	// read from block.
	else {
    openPageFile(bm->pageFile, &fh);
    
    if (fh.totalNumPages < pageNum) {
      ensureCapacity(pageNum+1, &fh);
    }
    readBlock(pageNum, &fh, ph);
		
		BM_PageHandle *newPageHandle = MAKE_PAGE_HANDLE();
		
		newPageHandle->pageNum = pageNum;
		newPageHandle->data = ph;
		
		Page_Frame *newPF = newPageFrame(pageNum, newPageHandle);
	
		printf("new mapping added for pageNum %d\n", pageNum);
		
		bs->mapping[pageNum] = newPF;
		
		pool->readIO++;

    // BM_PageHandle *pageHandle = MAKE_PAGE_HANDLE();
    // 
    // pageHandle->pageNum = pageNum;
    // pageHandle->data = ph;
    
		printf("ph is %s\n", ph);
    closePageFile(&fh);
    // return pageHandle;
		p->data = ph;
		p->pageNum = pageNum;
	}
	// BM_PageHandle *loadedPage = loadFromPageFile(bm->pageFile, pageNum);
	
	// pageNum is not found
	
	
	if (isPoolFull(bm)) {
		printf("it's full\n");
		if (bm->strategy == RS_FIFO) {
			replaced = replaceByFIFO(bm, removed, added);
			if (replaced > -1 && bs->mapping[replaced]->is_dirty){
				printf("removed is %d\n", replaced);
				printf("goes here is =1\n");
				
				// printf("we have dirty page-%d\n", removed->pageHandle->pageNum);
				// writeToDisk(bm, removed);
				// write to disk.
				SM_FileHandle fHandle;
				CHECK(openPageFile(bm->pageFile, &fHandle));
				CHECK(writeBlock(replaced, &fHandle, bs->mapping[replaced]->pageHandle->data));
				// free(removed);
				pool->writeIO++;
				CHECK(closePageFile( &fHandle));
				
				bs->mapping[replaced] = NULL;
				
			}
			else {
				printf("not dirty\n");
			}
		}
		else if (bm->strategy == RS_LFU) {
			// LRU;
		}
		else {}
	}
	else {
		
		if (pool->count == 0) {
			// pool is empty;
			
			pool->front = added;
			pool->rear = added;
			pool->count++;
			
		}
		else {
			pool->rear->next = added;
			added->prev = pool->rear;
			pool->rear = added;
			pool->count++;
		}
	}
	
	// added new mapping.
	// printf("rear %d\n", pool->rear->pageHandle->pageNum);
	// printf("middle %d\n", pool->front->next->pageHandle->pageNum);
	// printf("removed is %d\n", removed->pageHandle->pageNum);
	// printf("added is %d\n", added->pageHandle->pageNum);
	// printf("added is %s\n", added->pageHandle->data);
	bs->mapping[pageNum] = added;
	
	
	// //printf out mapping.
	// for (size_t i = 0; i < 20; i++) {
	// 	printf("address of page i=%d is %p\n", i, bs->mapping[i]);
	// 	/* code */
	// }

	
	
	//assinge new to pageHandle;
	
	page->pageNum = added->pageHandle->pageNum;
	page->data = added->pageHandle->data;
	
	// printf("h is %s\n", page->data);
	
	// free(added);
	
	// printf("====pool====\n");
	// for (size_t i = 0; i < 25; i++) {
	// 	printf("mapping[%d] is %p\n", i, bs->mapping[i]);
	// }
	
	// printf("pool-0\n", pool->front->next->pageHandle->pageNum);
	// printf("pool-2\n", pool->rear->prev->pageHandle->pageNum);
	
	// printf("====pool====\n");
	return RC_OK;
	
}


RC markDirty(BM_BufferPool * const bm, BM_PageHandle * const page)
{
	Buffer_Storage *bs = (Buffer_Storage *)bm->mgmtData;
	
	// printf("mark as dirty is page %d\n", page->pageNum);
	// printf("current address is %p\n", bs->mapping[page->pageNum]);
	Page_Frame *pf;
	BM_PageHandle *ph = MAKE_PAGE_HANDLE();
	ph->data = page->data;
	ph->pageNum = page->pageNum;
	
	

	if (bs->mapping[page->pageNum]) {
		pf = bs->mapping[page->pageNum];
		pf->pageHandle = ph;
		pf->is_dirty = true;
		return RC_OK;
	} 
	else {
		// error.
		printf("gose here error\n");
		return -1;
	}
}

RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page){
	
	Buffer_Storage *bs = (Buffer_Storage *)bm->mgmtData;
	
	Page_Frame *pf;
	BM_PageHandle *ph = MAKE_PAGE_HANDLE();
	ph->data = page->data;
	ph->pageNum = page->pageNum;
	printf("unpin page num is %d\n", ph->pageNum);
	printf("data in page is %s\n", ph->data);
	printf("bs address %p\n", bm);
	Queue *q = bs->pool;
	if (bs->mapping[page->pageNum]) {
		// Page_Frame *temp = q->front; 
		// while(temp!=NULL){
		// 	printf("temp page num %d\n", temp->pageHandle->pageNum);
		// 	if(temp->pageHandle->pageNum == page->pageNum){
		// 		printf("found iin pool %d\n", temp->pageHandle->pageNum);
		// 		temp->fix_count--;
		// 		printf("fix count now is %d\n", temp->fix_count);
		// 		// return rc_cannot_shutdown;
		// 	}
		// if(temp->is_dirty == true ){
		// 	printf("temp is %s\n", temp->pageHandle->data);
		// 	CHECK(writeBlock(temp-> pageHandle->pageNum, &fHandle, temp-> pageHandle->data));
		// 	temp->is_dirty = FALSE;
		// 	q->writeIO++;
		// 	
		// }
		// else {
		// 	printf("left is not dirty\n");
		// 	printf("temp is %s\n", temp->pageHandle->data);
		// }
		// 	temp = temp->next;
		// }
	// 	//TODO fix_count is 0;
	// 	// printf("in unpin%s\n", bs->mapping[page->pageNum]->fix_count);
		bs->mapping[page->pageNum]->fix_count--;
		return RC_OK;
	} 
	else {
		// error.
		printf("gose here error\n");
		return -1;
	}
	return RC_OK;
}

PageNumber *getFrameContents (BM_BufferPool *const bm) {
	Buffer_Storage *bs = (Buffer_Storage *)bm->mgmtData;
  PageNumber *arrnumP1 = (PageNumber *)malloc(bm->numPages * sizeof(PageNumber));
	Queue *q = bs->pool;
	Page_Frame *p = q->front;
	int i=0;
	
	// while(p!= NULL){		
	//     arrnumP1[i] = p->pageHandle->pageNum;
	// 		p = p->next;
	// 		i++;
  // }
  return arrnumP1;
}


bool *getDirtyFlags (BM_BufferPool *const bm)
{
  int index = 0;
  Buffer_Storage *md = (Buffer_Storage*)bm->mgmtData;
  Queue *q = md -> pool;
  bool *count = (bool *)malloc(sizeof(bool)*bm->numPages);
  Page_Frame *temp = q->front;
  while(temp!=NULL){
		printf("temp is dirty %d, pagenum is %d\n", temp->is_dirty, temp->pageHandle->pageNum);
	count[index++]=temp->is_dirty;
	temp = temp->next;
	
	
  Page_Frame *t2 = q->front;
	
	printf("=====\n");
	printf("pool size is %d\n", q->q_capacity);
	// while(t2!=NULL) {
		// printf("pool pageNum is %d\n", t2->pageHandle->pageNum);
		// printf("pool pageNum is dirty=%d\n", t2->is_dirty);
		
		// printf("pool-0 is %d\n", t2->pageHandle->pageNum);
		// printf("pool-1 is %d\n", t2->next->pageHandle->pageNum);
		// printf("pool-2 is %d\n", t2->next->next->pageHandle->pageNum);
		// 
		// 
		// t2 = t2->next;
	// }
	printf("=====\n");
  }

 return count;
}

int *getFixCounts (BM_BufferPool *const bm)
{
  int index = 0;
  Buffer_Storage *md = (Buffer_Storage*)bm->mgmtData;
  Queue *q = md -> pool;
  int *count = (int *)malloc(sizeof(int)*bm->numPages);
  Page_Frame *temp = q->front;
    while(temp!=NULL){
	count[index++]=temp-> fix_count;
	temp = temp->next;
  }
  return count;
}

int getNumReadIO (BM_BufferPool *const bm)
{
  Buffer_Storage *md = (Buffer_Storage*)bm->mgmtData;
  Queue *q = md -> pool;
  return q-> readIO;
}

int getNumWriteIO (BM_BufferPool *const bm)
{
  Buffer_Storage *md = (Buffer_Storage*)bm->mgmtData;
  Queue *q = md -> pool;
  return q-> writeIO;
}
