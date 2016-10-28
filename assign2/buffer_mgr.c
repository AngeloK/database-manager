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