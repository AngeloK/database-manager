#ifndef TABLE_MGR_H
#define TABLE_MGR_H

#include "tables.h"
#include "dberror.h"
#include "dt.h"
#include "buffer_mgr.h"


typedef struct Table_Header {
	int tableCapacity;
	int pageCount;
	char *lastAccessed;
	// int maxRecords;
	int recordsPerPage;
	// int *offsets;
	RID *freePointer;
	// BM_BufferPool *bm;
} Table_Header;


typedef struct Page_Header {
	int pageId;
	bool isFull;    // true or false;
	int freeSlot;  // -1 if the page is full.
	int recordCount;
	int recordCapacity;
	// struct Page_Header *prev;
	// struct Page_Header *next;
	// RID **mapping;
} Page_Header;

//Table header functions.
RC initTableManager(Table_Header *manager, Schema *schema);
RC readTableInfo(Table_Header *tableHeader, char *data);
RC writeTableInfo(Table_Header *tableHeader);

RC writeRecordToSlot(Table_Header *tableHeader, Record *record);

//page header functions.
RC readPageInfo(Page_Header *pageHeader, char *data);
RC writePageInfo(Page_Header *pageHeader);


#endif // RECORD_MGR_H
