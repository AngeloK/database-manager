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
	// initStorageManager();
	// char *testName = "";
	// BM_BufferPool *bm = MAKE_POOL();
  // testName = "Creating and Reading Back Dummy Pages";
  // CHECK(createPageFile("testbuffer.bin"));
  // createDummyPages(bm, 22);
  // checkDummyPages(bm, 20);
	// shutdownBufferPool(bm);
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