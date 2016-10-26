#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include "buffer_mgr.h"
#include "dberror.h"
#include "test_helper.h"

BM_BufferPool *bm;
BM_PageHandle *h;




// Buffer Manager Interface Pool Handling
RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, const int numPages, ReplacementStrategy strategy, 
		  void *stratData) {
		
		bm->pageFile = pageFileName;
		bm->numPages = numPages;
		bm->strategy = strategy;
		
		// the data structure of storage.
		return RC_OK;
};


RC shutdownBufferPool(BM_BufferPool *const bm) {
	// if there a a pinned page. throw a error
	
	// if there are dirty files, update the changes on disk.
	//forceFlushPool(bm);
	// else
	
	// free all variable (or structure).
	return RC_OK;
		
}
RC forceFlushPool(BM_BufferPool *const bm) {
	
	// check dirty bits.
	
}

// // Buffer Manager Interface Access Pages
// RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page) {
// 	
// 	
// }
// RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page){
// 	
// }
// RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page) {
// 	
// 	
// }
// RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, 
// 	    const PageNumber pageNum) {
// 			
// }
// 
// // Statistics Interface
// PageNumber *getFrameContents (BM_BufferPool *const bm);
// bool *getDirtyFlags (BM_BufferPool *const bm);
// int *getFixCounts (BM_BufferPool *const bm);
// int getNumReadIO (BM_BufferPool *const bm);
// int getNumWriteIO (BM_BufferPool *const bm);
