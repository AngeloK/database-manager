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
    bm->mgmtData = (Buffer_Storage *)initBufferStorage(pageFileName, numPages);
    
  return RC_OK;
}
// RC shutdownBufferPool(BM_BufferPool *const bm) {
// 	  SM_FileHandle fHandle;
// 	  Buffer_Storage *md = (Buffer_Storage*)bm->mgmtData;
// 	  Queue *q = md -> pool;
// 	  CHECK(openPageFile(bm->pageFile, &fHandle))
// 	  PageFrame *temp = q->front;
// 	  while(temp!=NULL){
// 	        if(temp-> fix_count>0){
// 	                return RC_CANNOT_SHUTDOWN;
// 	        }
// 	        if(temp->is_dirty == TRUE ){
// 	                CHECK(writeBlock(temp->pageNumber, &fHandle, temp->ph.data));
// 	                temp->is_dirty = FALSE;
// 	                q->writeIO++;
// 
// 	        }
// 	        temp = temp->next;
// 	  }
// 
// 
// 
// 	  free(q->front);
// 	  free(q->rear);
// 	  free(q);
// 	  free(md->apping);
// 	  free(md);
// 
// 	  Return RC_OK;
//   
//   return RC_OK;
// }
RC forceFlushPool(BM_BufferPool *const bm) {
  return RC_OK;
}

RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, 
	    const PageNumber pageNum) {

	// pageNum is found in mapping table.
	Buffer_Storage *bs = (Buffer_Storage *)bm->mgmtData;
	if (bs->mapping[pageNum]) {
		page->pageNum = pageNum;
		page->data = ((bs->mapping[pageNum]))->pageHandle->data;
		return RC_OK;
	}
	
	// BM_PageHandle *loadedPage = loadFromPageFile(bm->pageFile, pageNum);
	
	// pageNum is not found
	Page_Frame *removed;
	BM_PageHandle *p = (BM_PageHandle *)page;
	Page_Frame *added  = newPageFrame(pageNum, p); 
	
	if (isPoolFull(bm)) {
		// if (repacement(bm, removed, added));
		//ReplacementStrategy.
	}
	else {
		//add pageNum to buffer pool.
		//enqueue.
	}
	
	
	if (removed->is_dirty) {
		// writeToDisk(removed);
		// write to disk.
	}
	
	//assinge new to pageHandle;
	page->pageNum = added->pageHandle->pageNum;
	page->data = added->pageHandle->data;
}
