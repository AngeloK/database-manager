#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include "table_mgr.h"
#include "storage_mgr.h"
#include "record_mgr.h"



RC initTableManager(Table_Header *manager, Schema *schema) {
	Table_Header *tm = (Table_Header *)malloc(sizeof(Table_Header));
	
	int schemaLen = schemaLength(schema);	

	// bm->tableCapacity = (PAGE_FILE_CAP - 1) * 
	tm->tableCapacity = 1000;
	tm->pageCount = 0;

	char *timer;
	currentTime(timer);
	tm->lastAccessed = timer;

	tm->recordsPerPage = 1000;
	tm->freePointer = NULL;

	manager = tm;

	return RC_OK;
}
