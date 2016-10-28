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
RC shutdownBufferPool(BM_BufferPool *const bm) {
  
  return RC_OK;
}
RC forceFlushPool(BM_BufferPool *const bm) {
  return RC_OK;
}

RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, 
	    const PageNumber pageNum) {

	// pageNum is found in mapping table.
	if (bm->mapping[pageNum]) {
		page->pageNum = pageNum;
		page->data = *bm->mapping[pageNum];
		return RC_OK;
	}
	
	// pageNum is not found
	Page_Frame *removed;
	BM_PageHandle *p = (BM_PageHandle *)page;
	Page_Frame *added  = newPageFrame(pageNum, p); 
	
	
	
	if (isPoolFull(bm)) {
		if (repacement(bm, removed, add));
		//ReplacementStrategy.
	}
	else {
		//add pageNum to buffer pool.
	}
	
	
			
}