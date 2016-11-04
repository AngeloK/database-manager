#ifndef TABLE_MGR_H
#define TABLE_MGR_H

#include "tables.h"
#include "dberror.h"
#include "dt.h"


typedef struct Table_Header {
	int recordCount;
	int pageCount;
	char *lastAccessed;
	int maxRecords;
	int recordsPerPage;
	int *offsets;
	RID *freePointer;
} Table_Header;


typedef struct Page_Header {
	int pageId;
	bool isDeleted;
	bool isFull;    // 1 is full.
	int freeSlot;  // -1 if the page is full.
	// struct Page_Header *prev;
	// struct Page_Header *next;
	RID **mapping;
} Page_Header;

//Table header functions.
RC initTableManager(Table_Header *manager, Schema *schema);
RC readTableInfo(Table_Header *tableHeader, char *data);
RC writeTableInfo(Table_Header *tableHeader);

RC writeRecordToSlot(Table_Header *tableHeader, Record *record)

//page header functions.
RC readPageInfo(Page_Header *pageHeader, char *data);
RC writePageInfo(Page_Header *pageHeader);


#endif // RECORD_MGR_H
