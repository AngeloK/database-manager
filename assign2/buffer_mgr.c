#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdbool.h>

#include "buffer_mgr.h"
#include "dberror.h"
#include "test_helper.h"
#include "buffer_controller.h"
#include "storage_mgr.h"

// Buffer Manager Interface Pool Handling
RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, const int numPages, ReplacementStrategy strategy, 
		  void *stratData) {
		
		bm->pageFile = (char *)pageFileName;
		bm->numPages = numPages;
		bm->strategy = strategy;
		bm->mgmtData = (Buffer_Storage *)initBufferStorage(numPages);
		return RC_OK;
};


RC shutdownBufferPool(BM_BufferPool *const bm) {
	// if there a a pinned page. throw a error
	
	// if there are dirty files, update the changes on disk.
	//forceFlushPool(bm);
	// else
	
	// free all variable (or structure).
	destroyBufferStorage(bm->mgmtData);
	return RC_OK;
		
};
RC forceFlushPool(BM_BufferPool *const bm) {
	
	// check dirty bits.
	return RC_OK;
};

// Buffer Manager Interface Access Pages
RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page) {
	Buffer_Storage *bs = (Buffer_Storage *)bm->mgmtData;

	// int bm_size = bm->numPages;
	// int i;
	// for (i = 0; i < bm_size; i++) {
	// 	if (bs->pool[i].pageNum == page->pageNum) {
	// 		bs->pool[i].is_dirty = true;
	// 		break;
	// 	}
	// }
	return RC_OK;
}
RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page){
	Buffer_Storage *bs = (Buffer_Storage *)bm->mgmtData;
	// int bm_size = bm->numPages;	
	// int i;
	// for (i = 0; i < bm_size; i++) {
	// 	if (bs->pool[i].pageNum == page->pageNum) {
	// 		bs->pool[i].fix_count--;
	// 		break;
	// 	}
	// }
	return RC_OK;
}
RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page) {
	
	
}
RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, 
	    const PageNumber pageNum) {
	Buffer_Storage *bs = (Buffer_Storage *)bm->mgmtData;
 // 	int bm_size = bm->numPages;	
	// int i;
	// printf("pagenum in pinpage is %d\n", pageNum);
	// printf("pointer 1\n");
	// for (i = 0; i < bm_size; i++) {
	// 	if (bs->pool[i].pageNum == pageNum) {
	// 		bs->pool[i].fix_count++;
	// 		page->data = bs->pool[i].data;
	// 		page->pageNum = bs->pool[i].pageNum;
	// 		return RC_OK;
	// 	}
	// }
	// page not found, replacement strategy is being used.
	printf("pointer 2\n");
	printf("bs-size=%d, bs-count=%d\n", bs->size, bs->count);
	if (isBufferStorageFull(bs)) {
			printf("pointer 4 if full\n");
			replacePageFrame(bm, page, pageNum);
			return RC_OK;
	}
	else {
		// printf("pointer 3\n");
		// for (i = 0; i < bm_size; i++) {
		// 	// if (strcmp(bs->pool[i].data, "") == 0) {
		// 	if (bs->pool[i].pageNum == NO_PAGE) {
		// 		replacePageFrame(bm, page, pageNum);
		// 		printPool(bs);
		// 		// printf("%d\n", page->pageNum);
		// 		// printf("%s\n", page->data);
		// 		return RC_OK;
		// 	}
		// }	
	}
	return -1;
};

// Statistics Interface
PageNumber *getFrameContents (BM_BufferPool *const bm) {
	Buffer_Storage *bs = (Buffer_Storage *)bm->mgmtData;
	int bm_size = bm->numPages;
	int *contents = (int *)malloc(sizeof(int) * bm_size);
	
	int i;
	for (i = 0; i < bm_size; i++) {
		contents[i] = bs->pool[i].pageHandle->pageNum;
	}
	return contents;
}
bool *getDirtyFlags (BM_BufferPool *const bm) {
	Buffer_Storage *bs = (Buffer_Storage *)bm->mgmtData;
	int bm_size = bm->numPages;
	bool *flags = (bool *)malloc(sizeof(bool) * bm_size);
	
	int i;
	for (i = 0; i < bm_size; i++) {
		flags[i] = bs->pool[i].is_dirty;
	}
	return flags;	
}
int *getFixCounts (BM_BufferPool *const bm) {
	Buffer_Storage *bs = (Buffer_Storage *)bm->mgmtData;
	int bm_size = bm->numPages;
	int *fix_counts = (int *)malloc(sizeof(int) * bm_size);
	
	int i;
	for (i = 0; i < bm_size; i++) {
		fix_counts[i] = bs->pool[i].fix_count;
	}
	return fix_counts;	
}
// int getNumReadIO (BM_BufferPool *const bm){
// 	Buffer_Storage *bs = (Buffer_Storage *)bm->mgmtData;
// 	int bm_size = bm->numPages;
// 	int *numReadIO = (int *)malloc(sizeof(int) * bm_size);
// 
// 	int i;
// 	for (i = 0; i < bm_size; i++) {
// 		numReadIO[i] = bs->pool[i].numReadIO;
// 	}
// 	return numReadIO;	
// }
// 
// int getNumWriteIO (BM_BufferPool *const bm) {
// 	Buffer_Storage *bs = (Buffer_Storage *)bm->mgmtData;
// 	int bm_size = bm->numPages;
// 	int *numWriteIO = (int *)malloc(sizeof(int) * bm_size);
// 
// 	int i;
// 	for (i = 0; i < bm_size; i++) {
// 		numWriteIO[i] = bs->pool[i].numWriteIO;
// 	}
// 	return numWriteIO;	
// }
