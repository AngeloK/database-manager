#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>

#include "buffer_mgr.h"
#include "dberror.h"
#include "storage_mgr.h"
#include "buffer_controller.h"

// #include "structures.h"


int main(int argc, char const *argv[]) {
	
	// time_t t0 = time(0);
	// printf("%ld\n", t0);
	
	// int *a = (int *)malloc(sizeof(int) * 3);
	// 
	// a[0] = 1;
	// 
	// printf("%d\n", a[0]);
	// if (strcmp(testName, "")) {
	// 	printf("yeah\n");
	// }
	// 
	BM_BufferPool *bm = MAKE_POOL();
	initBufferPool(bm, "testbuffer.bin", 3, RS_FIFO, NULL);
	// 
  BM_PageHandle *h = MAKE_PAGE_HANDLE();
	// printf("%d\n", h->pageNum);
	pinPage(bm, h, 1);
	sprintf(h->data, "%s-%i", "Page", h->pageNum);
	
	printf("%s\n", h->data);
	
	// CHECK(markDirty(bm, h));
	shutdownBufferPool(bm);
	// getFrameContents(bm);
	
	// BM_BufferPool *bm = MAKE_POOL();
	// BM_PageHandle *h = MAKE_PAGE_HANDLE();
	// 
	// 
	// // BM_PageFrames *bfp = BM_PageFrames;
	// 
	// char *numstring = (char*) malloc(80);
	// initStorageManager();
	// createPageFile(testName);
	// 
	// 
	// free(h);
	return 0;
}