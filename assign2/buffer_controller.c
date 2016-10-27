#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include "buffer_controller.h"
#include "storage_mgr.h"
// #include "buffer_mgr.h"


Buffer_Storage *initBufferStorage(int size) {
  Buffer_Storage *bs = (Buffer_Storage *)malloc(sizeof(Buffer_Storage));
  bs->size = size;
  bs->count = 0;
  bs->curPos = 0;
  bs->numReadIO = 0;
  bs->numWriteIO = 0;
  bs->pool = (Page_Frame *)malloc(sizeof(Page_Frame) * size);
  
  int i;
  for (i = 0; i < size; i++) {
    BM_PageHandle *pageHandle;
    bs->pool[i].fix_count = 0;
    bs->pool[i].is_dirty = false;
    bs->pool[i].index = -1;
    bs->pool[i].pageHandle = pageHandle;
  }
  return bs;
}

void destroyBufferStorage(Buffer_Storage *bs) {
  printf("going to destroy it\n");
  free(bs->pool);
  free(bs);
};

// void replacePageFrame(ReplacementStrategy strategy, Buffer_Storage *bs, char *pageFileName, BM_PageHandle *page, PageNumber pageNum, int index) {
RC replacePageFrame(ReplacementStrategy strategy, BM_BufferPool *bm, BM_PageHandle *pagehandle) {
  if (strategy == RS_FIFO) {
    // printf("page num in replacePageFrame is %d\n", pageNum);
    // Page_Frame newPF;
    // newPF.fix_count = 1;
    // newPF.is_dirty = false;
    // newPF.pageNum = pageNum;
    // newPF.last_access = 0;
    // newPF.data = (char *)malloc(PAGE_SIZE);
    // 
    // SM_FileHandle fHandle;
    // 
    // openPageFile(pageFileName, &fHandle); 
    // ensureCapacity(pageNum + 1, &fHandle); /*to ensure all pages are there n file - assuming my page number has correct page number has total number of pages. */
    // readBlock(pageNum, &fHandle, newPF.data); /* read data for files to pageframe’s page handle data */
    // closePageFile(&fHandle);
    // 
    // page->data = newPF.data;
    // printf("pageNum=%d\n",pageNum);
    // page->pageNum = pageNum;
    // bs->pool[index] = newPF;
    // bs->curPos++;
    // bs->count++;
  }
}

void printPool(Buffer_Storage *bs) {
  // for (size_t i = 0; i < bs->size; i++) {
    /* code */
    // printf("i-th is: %d\n", bs->pool[i].pageNum );
    // printf("i-th data is: %s\n", bs->pool[i].data);
  // }
}

