#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include "buffer_controller.h"


Buffer_Storage *initBufferStorage(int size) {
  Buffer_Storage *bs = (Buffer_Storage *)malloc(sizeof(Buffer_Storage *));
  bs->size = size;
  bs->count = 0;
  bs->curPos = 0;
  bs->numReadIO = 0;
  bs->numWriteIO = 0;
  bs->pool = (Page_Frame *)malloc(sizeof(Page_Frame) * size);
  
  int i;
  for (i = 0; i < size; i++) {
    bs->pool[i].fix_count = 0;
    bs->pool[i].is_dirty = false;
    bs->pool[i].pageNum = -1;
    bs->pool[i].last_access = 0;
    bs->pool[i].data = "";
  }
  return bs;
}

void destroyBufferStorage(Buffer_Storage *bs) {
  printf("going to destroy it\n");
  free(bs->pool);
  free(bs);
};

void updatePageFrame(Page_Frame *pf, BM_PageHandle *const page) {
  pf->fix_count = 1;
  pf->is_dirty = false;
  pf->pageNum = page->pageNum;
  pf->last_access = 0;
  pf->data = page->data;
}

