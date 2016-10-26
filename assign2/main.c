#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include "buffer_mgr.h"
#include "dberror.h"
#include "storage_mgr.h"

char *testName = "test.bin";

int main(int argc, char const *argv[]) {
	
	BM_BufferPool *bm = MAKE_POOL();
	BM_PageHandle *h = MAKE_PAGE_HANDLE();
	
	initStorageManager();
	createPageFile(testName);
	
	int i;

	initBufferPool(bm, "test.bin", 3, RS_FIFO, NULL);
	
	for (i = 0; i < num; i++)
		{
			CHECK(pinPage(bm, h, i));
			sprintf(h->data, "%s-%i", "Page", h->pageNum);
			CHECK(markDirty(bm, h));
			CHECK(unpinPage(bm,h));
		}

	CHECK(shutdownBufferPool(bm));

	free(h);
	return 0;
}