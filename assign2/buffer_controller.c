#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include "buffer_controller.h"
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
    
    printf("i=%d\n", i);
    bs->pool[i].fix_count = 0;
    bs->pool[i].is_dirty = false;
    bs->pool[i].pageNum = -1;
    bs->pool[i].last_access = 0;
    bs->pool[i].data = "";
    printf("page num =%d\n", bs->pool[i].pageNum);
  }
  return bs;
}

void destroyBufferStorage(Buffer_Storage *bs) {
  printf("going to destroy it\n");
  free(bs->pool);
  free(bs);
};

void replacePageFrame(ReplacementStrategy strategy, Buffer_Storage *bs, BM_PageHandle *page, PageNumber pageNum, int index) {
  if (strategy == RS_FIFO) {
    
    Page_Frame newPF;
    
    newPF.fix_count = 1;
    newPF.is_dirty = false;
    newPF.pageNum = pageNum;
    newPF.last_access = 0;
    newPF.data = (char *)malloc(PAGE_SIZE);
    
    page->data = newPF.data;
    page->pageNum = pageNum;
    bs->pool[index] = newPF;
  }
}

void printPool(Buffer_Storage *bs) {
  for (size_t i = 0; i < bs->size; i++) {
    /* code */
    printf("i-th is: %d\n", bs->pool[i].pageNum );
    printf("i-th data is: %s\n", bs->pool[i].data);
  }
}

