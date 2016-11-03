#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

// #include "storage_mgr.h"
#include "dberror.h"
#include "test_helper.h"
// #include "buffer_pool.h"


// table and manager
RC initRecordManager (void *mgmtData) {
  return RC_OK;
}
RC shutdownRecordManager () {
  //free memeory.
}
RC createTable (char *name, Schema *schema) {
  // here just create a table with a name.
}
RC openTable (RM_TableData *rel, char *name) {
  // Open a table via table name.
}
RC closeTable (RM_TableData *rel);
RC deleteTable (char *name);
int getNumTuples (RM_TableData *rel);


