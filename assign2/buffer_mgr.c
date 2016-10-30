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
	
	printf("## pinPage is %d\n##", pageNum);
	Buffer_Storage *bs = (Buffer_Storage *)bm->mgmtData;
	
  SM_FileHandle fh;
  SM_PageHandle ph;
  ph = (SM_PageHandle) malloc(PAGE_SIZE);
	BM_PageHandle *p = MAKE_PAGE_HANDLE();
	int replaced;
	Page_Frame *removed;
	Page_Frame *added = NULL; 
	Queue *pool = bs->pool;
	

	// printQueueElement(pool);
	
	// read from mapping.
	if (bs->mapping[pageNum]) {
		// page->pageNum = pageNum;
		// printf("bs->mapping->pageHandle->data=%s\n", bs->mapping[pageNum]->pageHandle->data);
		// printf("page->data %p\n", page->data);
		// page->data = (bs->mapping[pageNum])->pageHandle->data;
			
		printf("found in mapping %d\n", pageNum);
		page->data = (bs->mapping[pageNum])->pageHandle->data;
		bs->mapping[pageNum]->fix_count++;
		// pool->readIO++;
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
		
		printf("ph is %s\n", ph);
		pool->readIO++;
		
		// BM_PageHandle *newPageHandle = MAKE_PAGE_HANDLE();
		
		p->pageNum = pageNum;
		p->data = ph;
		
		
		added = newPageFrame(pageNum, p);
		
	
		printf("new mapping added for pageNum %d\n", pageNum);
		
		bs->mapping[pageNum] = added;
		
		
		// free(newPageHandle);

    // BM_PageHandle *pageHandle = MAKE_PAGE_HANDLE();
    // 
    // pageHandle->pageNum = pageNum;
    // pageHandle->data = ph;
    
    closePageFile(&fh);
    // return pageHandle;
		// p->data = ph;
		// p->pageNum = pageNum;
	}
	// BM_PageHandle *loadedPage = loadFromPageFile(bm->pageFile, pageNum);
	
	// pageNum is not found
	
	
	if (bm->strategy == RS_FIFO) {
		printf("added pageNum %d\n", added->pageHandle->pageNum);
		replaced = Replacement(bs->pool, bs->mapping, removed, added);
		printf("replace is %d\n", replaced);
		if (replaced != -1){
			printf("is dirty %d-%d\n", bs->mapping[replaced]->is_dirty, replaced);
			if ((bs->mapping[replaced])->is_dirty){
				printf("removed is %d\n", replaced);
				printf("goes here is =1\n");
				
				// printf("we have dirty page-%d\n", removed->pageHandle->pageNum);
				// writeToDisk(bm, removed);
				// write to disk.
				
				SM_FileHandle fHandle;
				CHECK(openPageFile(bm->pageFile, &fHandle));
				CHECK(writeBlock(replaced, &fHandle, bs->mapping[replaced]->pageHandle->data));
				// free(removed);
				CHECK(closePageFile( &fHandle));
				pool->writeIO++;
				
			}
			else {
				printf("mapping address if page %d is %p\n", bs->mapping[replaced]->is_dirty, bs->mapping[replaced]);
				printf("not dirty\n");
			}
			
			//
			if (bs->mapping[replaced]->fix_count == 0) {
				printf("replace is %d it's removeing from mapping\n", replaced);
				bs->mapping[replaced] = NULL;
			}
		}
	}
	else {
		//LRU
	}
	
	printQueueElement(pool);
	// bs->mapping[pageNum] = added;
	
	page->pageNum = added->pageHandle->pageNum;
	page->data = added->pageHandle->data;
	
	
	
	
	
	// free(ph);
	
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
		printf("mark pageNum %d dirty\n", page->pageNum);
		pf = bs->mapping[page->pageNum];
		pf->pageHandle = ph;
		pf->is_dirty = true;
		return RC_OK;
	} 
	else {
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
		bs->mapping[page->pageNum]->fix_count--;
		printQueueElement(bs->pool);
		printf("fix count after unpin is %d (page=%d)\n", bs->mapping[page->pageNum]->fix_count, page->pageNum);
		// if (bs->mapping[page->pageNum]->fix_count == 0)
		//   // remove mapping.
		// 	bs->mapping[page->pageNum] = NULL;
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
	Queue *pool = bs->pool;
	Page_Frame *temp = pool->front;
	printQueueElement(pool);
	
	int i=0;
	if (pool->count < pool->q_capacity){	
		while(temp!= NULL){		
	    arrnumP1[i] = temp->pageHandle->pageNum;
			temp = temp->next;
			i++;
	  }
		
		int idx;
		for (idx = i; idx < pool->q_capacity; idx++) {
			arrnumP1[idx] = NO_PAGE;
		}
	}	
	else  {
		temp = pool->front;
		while (temp) {
			arrnumP1[temp->index] = temp->pageHandle->pageNum;
			temp = temp->next;
		}
		
	}
	free(temp);
	return arrnumP1;
}


bool *getDirtyFlags (BM_BufferPool *const bm)
{
  int index = 0;
  Buffer_Storage *bs = (Buffer_Storage*)bm->mgmtData;
  bool *dirtyFlags = (bool *)malloc(sizeof(bool)*(bm->numPages));
	// bool dirtyFlags[bm->numPages];
	Queue *pool = bs->pool;
  Page_Frame *temp = pool->front;
	Page_Frame **mapping = bs->mapping;
	
	int i=0;
	if (pool->count < pool->q_capacity){	
		while(temp!= NULL){		
	    dirtyFlags[temp->index] = temp->is_dirty;
			temp = temp->next;
			i++;
	  }
		int idx;
		for (idx = i; idx < pool->q_capacity; idx++) {
			dirtyFlags[idx] = false;
		}
	}
	else  {
		// while (temp) {
		// 	// printf("temp index is %d value is %d address is %p\n", temp->index, temp->is_dirty, temp);
		// 	// dirtyFlags[temp->index] = temp->is_dirty;
		// 	dirtyFlags[temp->index] = mapping[temp->pageHandle->pageNum]->is_dirty;
		// 	temp = temp->next;
		// 	i--;
		// }
		printQueueElement(pool);
		printf("front next is %d\n", pool->front->next->is_dirty);
		printf("rear is %d\n", pool->rear->is_dirty);
		dirtyFlags[pool->front->index] = pool->front->is_dirty;
		dirtyFlags[pool->front->next->index] = pool->front->next->is_dirty;
		dirtyFlags[pool->rear->index] = pool->rear->is_dirty;
		}

		printf("dirtyFlags[1] = %d\n", dirtyFlags[1]);
	// }
	// printQueueElement(pool);
	int j;
	for (j = 0; j < 3; j++) {
		// dirtyFlags[j] = 100;
		printf("dirtyflags[%d] %d value pointer is %p\n",j, dirtyFlags[j], &dirtyFlags[j]);
		/* code */
	}
	// free(temp);
 return &dirtyFlags[0];
}

int *getFixCounts (BM_BufferPool *const bm)
{
  int index = 0;
  Buffer_Storage *bs = (Buffer_Storage*)bm->mgmtData;
  int *fixCount = (int *)malloc(sizeof(int)*bm->numPages);
	Queue *pool = bs->pool;
  Page_Frame *temp = pool->front;
	
	int i=0;
	if (pool->count < pool->q_capacity){	
		while(temp!= NULL){		
	    fixCount[i] = temp->fix_count;
			temp = temp->next;
			i++;
	  }
		
		int idx;
		for (idx = i; idx < pool->q_capacity; idx++) {
			fixCount[idx] = 0;
		}
	}	
	else  {
		temp = pool->front;
		while (temp) {
			fixCount[temp->index] = temp->fix_count;
			temp = temp->next;
		}
	}
	free(temp);
 return fixCount;
}

int getNumReadIO (BM_BufferPool *const bm)
{
  Buffer_Storage *bs = (Buffer_Storage*)bm->mgmtData;
  Queue *pool = bs -> pool;
  return pool->readIO;
}

int getNumWriteIO (BM_BufferPool *const bm)
{
  Buffer_Storage *bs = (Buffer_Storage*)bm->mgmtData;
  Queue *pool = bs -> pool;
  return pool->writeIO;
}
