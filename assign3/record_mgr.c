#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>

#include "record_mgr.h"
#include "storage_mgr.h"
#include "dberror.h"
#include "test_helper.h"
#include "buffer_mgr.h"
#include "buffer_pool.h"
#include "tables.h"
#include "expr.h"
#include "rm_serializer.c"
// #include "table_mgr.h"

// table and manager
RC initRecordManager (void *mgmtData) {

  return RC_OK;
}
RC shutdownRecordManager () {
  //free memeory.
	return RC_OK;
}
RC createTable (char *name, Schema *schema) {
  RM_TableData *table = (RM_TableData *) malloc(sizeof(RM_TableData));
	Page_Header *pegeHeader = (Page_Header *)malloc(sizeof(Page_Header));

	table->name = name;
	table->schema = schema;

	Table_Header *tableHeader = (Table_Header *)malloc(sizeof(Table_Header));
	initTableManager(tableHeader, schema);
	table->mgmtData = tableHeader;

  BM_BufferPool *bm = MAKE_POOL();
  BM_PageHandle *h = MAKE_PAGE_HANDLE();
  SM_FileHandle fh;

  createPageFile(name);

  initBufferPool(bm, name, 5, RS_FIFO, NULL);

	pinPage(bm, h, 0);
	// add header data into pagefile.
	h->data = generateTableInfo(table);
	markDirty(bm, h);
	unpinPage(bm, h);

	// add page header data into the first page;
	pinPage(bm, h, 1);

  // TODO replace 'null' with pageHeader.
	Page_Header *pageHeader = (Page_Header *)malloc(sizeof(Page_Header));

	initPageHeader(table, pageHeader, 1);
	h->data = generatePageHeader(table, pageHeader);
	markDirty(bm, h);
	unpinPage(bm, h);

	shutdownBufferPool(bm);
	free(h);

  // openPageFile(name, &fh);

  // insert header pages based on the schema.
  /* those pages may include
    1.schema;
    2.records count;
    3.free space pointer (page_id, slot_id);
    4.number of records 1 block can save;
    5.An RID array save all RIDs.
    6.May have status controller like 0 is closed, 1 is opened.
  */

  // Init every parameter.
  // record = 0

	shutdownBufferPool(bm);
  closePageFile(&fh);
  return RC_OK;

  // here just create a table with a name.
}
RC openTable (RM_TableData *rel, char *name) {
  // Open a table via table name.
  rel->name = name;
	//
	SM_FileHandle fh;
	SM_PageHandle ph;
	ph = (SM_PageHandle) malloc(PAGE_SIZE);

	openPageFile(name, &fh);
	readBlock(0, &fh, ph);

	rel->schema = (Schema *)malloc(sizeof(Schema));
	parseTableHeader(rel, ph);
	printf("schema in openTable is :%s\n", serializeSchema(rel->schema));

	return RC_OK;
}


RC closeTable (RM_TableData *rel) {

  return RC_OK;

}
RC deleteTable (char *name) {
  destroyPageFile(name);
  return RC_OK;
}
int getNumTuples (RM_TableData *rel) {
	Table_Header *tableHeader = (Table_Header *)rel->mgmtData;
  return tableHeader->totalRecordCount;
  return 0;
}


RC insertRecord (RM_TableData *rel, Record *record) {
  // if inside the record, we have the RID.
  if (record->id.page) {
    // update record based on rid;
  }
  else {
    // insert a new record;
    // assign rid to record->rid;
  }
  return RC_OK;
}
RC deleteRecord (RM_TableData *rel, RID id) {
  // define r;
  // getRecord(rel, id, r);
  // mark r as deleted.
  return RC_OK;
}
RC updateRecord (RM_TableData *rel, Record *record) {
  // define a new r;
  // getRecord(rel, record->id, r);
  // r now is a pointer points to record.

  // apply record to r.
  // save updated record:
  // insertRecord(rel, record)

	return RC_OK;
}
RC getRecord(RM_TableData *rel, RID id, Record *record) {
  // if the record has been delted (detect by record->status == DELETED).
  // record = NULL;
  // else
  // assign r = record;
  // pageMapping = getPageMapping(rel, id->page);

  // r = pageMapping[id->slot];
  // record = r;
  return RC_OK;
}


// scans
RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond) {
	return RC_OK;
}
RC next (RM_ScanHandle *scan, Record *record) {
	return RC_OK;
}
RC closeScan (RM_ScanHandle *scan) {
	return RC_OK;
}

// dealing with schemas
int getRecordSize (Schema *schema) {
	return 0;
}
Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys) {
	Schema *schema = (Schema *)malloc(sizeof(Schema));

	schema->numAttr = numAttr;
	schema->attrNames = attrNames;
	schema->dataTypes = dataTypes;
	schema->typeLength = typeLength;
	schema->keyAttrs = keys;
	schema->keySize = keySize;
	return schema;
}

RC freeSchema (Schema *schema) {
	return RC_OK;
}


int schemaLength(Schema *schema) {
	// get the length of record->data by add the length of each data type.
	int schemaLength= 0;
  int i;
  for (i = 0; i < schema->numAttr; i++) {
		if (schema->dataTypes[i] == DT_STRING) {
		schemaLength+= schema->typeLength[i];
		}
		else {
		schemaLength+= 4;
		}
  }
	return schemaLength;
}


// dealing with records and attribute values
RC createRecord (Record **record, Schema *schema) {
	// Reference for why use double pointer here.
	// http://stackoverflow.com/questions/5580761/why-use-double-pointer-or-why-use-pointers-to-pointers
	int recordLength = schemaLength(schema);

	*record = (Record *)malloc(sizeof(Record));

	(*record)->data = (char *)malloc(recordLength);

  return RC_OK;
}
RC freeRecord (Record *record) {
	free(record);
	return RC_OK;
}

RC getAttr (Record *record, Schema *schema, int attrNum, Value **value) {
	*value = (Value *)malloc(sizeof(Value));
	int offset;
	attrOffset(schema, attrNum, &offset);
	char *valueFromRecord = record->data + offset;

	switch (schema->dataTypes[attrNum]) {
		case DT_INT:
			(*value)->dt = DT_INT;
			(*value)->v.intV = atoi(valueFromRecord); // get int value from string.
			break;
		case DT_STRING:
			(*value)->dt = DT_STRING;
			int stringLength = schema->typeLength[attrNum];

			// Allocate space.
			(*value)->v.stringV = (char *)malloc(stringLength);
			strncpy((*value)->v.stringV, valueFromRecord, stringLength);
			break;
		case DT_BOOL:
			(*value)->dt = DT_BOOL;

			// TODO assign bool value.
			// (*value)->v.boolV = transferTo(valueFromRecord, DT_BOOL);
			// (*value)->v.boolV = NULL;
			break;
		case DT_FLOAT:
			(*value)->dt = DT_FLOAT;
			// TODO assign float value.
			// (*value)->v.floatV = NULL;
			break;
	}

	// if ((*value)->dt == DT_STRING) {
	// 	printf("value final is %s\n", (*value)->v.stringV );
	// }
  return RC_OK;
}
RC setAttr (Record *record, Schema *schema, int attrNum, Value *value) {
	int offset;
	char *result = serializeValue(value);

	if (attrOffset(schema, attrNum, &offset) == 0) {
			strcpy(record->data+offset, result);
		  return RC_OK;
	}
	else {
		// error.
		return -1;
	}

}


int currentTime(char *buffer) {
	time_t timer;
	char *t = (char *)malloc(26);
	struct tm* tm_info;

	time(&timer);
	tm_info = localtime(&timer);

	strftime(t, 19, "%Y-%m-%d %H:%M:%S", tm_info);
	strcpy(buffer, t);
	free(t);

	return 0;
}

// implementations
char *generateTableInfo(RM_TableData *rel) {
  VarString *result;
  MAKE_VARSTRING(result);
	Schema *schema = rel->schema;
	Table_Header *tableHeader = (Table_Header *)rel->mgmtData;
	char *timer = (char *)malloc(26);
	// char *r;

	int recordLen = schemaLength(rel->schema);
	// int recordLen = 26;

	APPEND(result, "%s", rel->name);
	// APPEND(result, "%s", "Hello_World");

	APPEND_STRING(result, "&");

	// tableCapacity.
	APPEND(result, "%d", tableHeader->tableCapacity);
	APPEND_STRING(result, "&");

	// pageCapacity.
	APPEND(result, "%d", tableHeader->recordsPerPage);
	APPEND_STRING(result, "&");

	// pageCount.
	APPEND(result, "%d", tableHeader->pageCount);
	APPEND_STRING(result, "&");

	// recordCount.
	// APPEND(result, "%d", getNumTuples(rel));
	APPEND(result, "%d", tableHeader->totalRecordCount);
	APPEND_STRING(result, "&");

	// lastAccessed.
	currentTime(timer);
	APPEND_STRING(result, timer);
	APPEND_STRING(result, "&");

	// APPEND_STRING(result, serializeSchema(rel->schema));

	// schema->numAttr.
	APPEND(result, "%d", rel->schema->numAttr);
	// APPEND(result, "%d", 3);
	APPEND_STRING(result, "&");

	char *dt[4] = {"DT_INT", "DT_STRING", "DT_BOOL", "DT_FLOAT"};

	int i;
	for (i = 0; i < schema->numAttr; i++) {
		APPEND_STRING(result, schema->attrNames[i]);
		APPEND_STRING(result, "&");
		switch (schema->dataTypes[i]) {
			case DT_INT:
				APPEND_STRING(result, "DT_INT");
				APPEND_STRING(result, "&");
				APPEND(result, "%d", 0);
				break;
			case DT_STRING:
				APPEND_STRING(result, "DT_STRING");
				APPEND_STRING(result, "&");
				APPEND(result, "%d", schema->typeLength[i]);
				break;
			case DT_BOOL:
				APPEND_STRING(result, "DT_BOOL");
				APPEND_STRING(result, "&");
				APPEND(result, "%d", 0);
				break;
			case DT_FLOAT:
				APPEND_STRING(result, "DT_FLOAT");
				APPEND_STRING(result, "&");
				APPEND(result, "%d", 0);
				break;
		}
	APPEND_STRING(result, "&");
	}
  RETURN_STRING(result);
}

int tableInfoLength(RM_TableData *rel) {
	int nameLen = strlen(rel->name);
	int maxRecordsLen = sizeof(int);
	int lastAccessedLen = 19; // 2016-11-04 21:10:00
	int schemaLen = schemaLength(rel->schema);
	int recordsPerPageLen = sizeof(int);
	int freespacePointerLen = sizeof(RID *);
	int total = nameLen + maxRecordsLen + lastAccessedLen + schemaLen + recordsPerPageLen + freespacePointerLen;
	return total;
}


char *generatePageHeader(RM_TableData *rel, Page_Header *pageHeader) {
	VarString *result;
	MAKE_VARSTRING(result);
	// TODO Complete page header functions.

	APPEND(result, "%d", pageHeader->pageId);
	// APPEND(result, "%d", 1);
	APPEND_STRING(result, "&");

	APPEND(result, "%d", pageHeader->isFull);
	// APPEND(result, "%d", 0);
	APPEND_STRING(result, "&");

	APPEND(result, "%d", pageHeader->recordCount);
	// APPEND(result, "%d", 100);
	APPEND_STRING(result, "&");

	APPEND(result, "%d", pageHeader->recordCapacity);
	// APPEND(result, "%d", 1000);

	RETURN_STRING(result);
}

RC initTableManager(Table_Header *manager, Schema *schema) {
	// Table_Header *tm = (Table_Header *)malloc(sizeof(Table_Header));

	int schemaLen = schemaLength(schema);

	manager->tableCapacity = (TOTAL_PAGES - 1) * ((PAGE_SIZE - 50)/schemaLen);
	manager->pageCount = 0;

	char *timer = (char *)malloc(26);
	currentTime(timer);
	manager->lastAccessed = timer;

	manager->recordsPerPage = (PAGE_SIZE - 50)/schemaLen;
	manager->freePointer = NULL;

	return RC_OK;
}

RC parseTableHeader(RM_TableData *rel, char *stringHeader) {

	Schema *schema;
	Table_Header *tableHeader = (Table_Header *)malloc(sizeof(Table_Header));

	char *tableAttrs[6];
	char *token;
	token = strtok(stringHeader, "&");
	int i = 0;
	while (token != NULL && i < 6)
	{
		tableAttrs[i] = token;
		token = strtok (NULL, "&");
		i++;
	}


	// gererate schem from string.
	int numAttr = atoi(token);
	token = strtok (NULL, "&");

	char *attrNames[numAttr];
	DataType dataTypes[numAttr];
	int typeLength[numAttr];
	int keyAttrs[numAttr];
	// TODO what's 'keySize'?
	int keySize = 1;
	int keys[] = {0};
	i = 0;

	for (i = 0; i < numAttr; i++) {
		int columnIdx = 0;

		while (token != NULL && columnIdx < 3)
		{
			switch (columnIdx) {
				case 0:
					attrNames[i] = token;
					break;
				case 1:
					dataTypes[i] = stringToDatatype(token);
					break;
				case 2:
					typeLength[i] = atoi(token);
					break;
			}
			token = strtok (NULL, "&");
			columnIdx++;
		}
	}

  char **cpNames = (char **) malloc(sizeof(char*) * numAttr);
  DataType *cpDt = (DataType *) malloc(sizeof(DataType) * numAttr);
  int *cpSizes = (int *) malloc(sizeof(int) * numAttr);
  int *cpKeys = (int *) malloc(sizeof(int));

  for(i = 0; i < 3; i++)
    {
      cpNames[i] = (char *) malloc(2);
      strcpy(cpNames[i], attrNames[i]);
    }
  memcpy(cpDt, dataTypes, sizeof(DataType) * 3);
  memcpy(cpSizes, typeLength, sizeof(int) * 3);
  memcpy(cpKeys, keys, sizeof(int));

	schema = createSchema(numAttr, cpNames, cpDt, cpSizes, 1, cpKeys);


	rel->name = tableAttrs[0];
	rel->schema = schema;
	// puts("hello");
	// printf("%s\n", "hello");
	printf("%s", serializeSchema(rel->schema));
	// printf("%p\n", rel->schema);


	tableHeader->tableCapacity = atoi(tableAttrs[1]);
	tableHeader->pageCount = atoi(tableAttrs[2]);
	tableHeader->totalRecordCount = atoi(tableAttrs[3]);
	tableHeader->recordsPerPage = atoi(tableAttrs[4]);
	tableHeader->lastAccessed = tableAttrs[5];

	rel->mgmtData = tableHeader;
	// printf("schema address %p\n", schema);
	// printf("%s\n", serializeSchema(schema));
	// printf("%s\n", serializeSchema(rel->schema));
	printf("[%s]\n", serializeSchema(rel->schema));
	printf("%p\n", rel->schema->attrNames);

	return RC_OK;
}

DataType stringToDatatype(char *token) {
	if (strcmp(token, "DT_INT") == 0) {
		return DT_INT;
	}

	if (strcmp(token, "DT_STRING") == 0) {
		return DT_STRING;
	}

	if (strcmp(token, "DT_BOOL") == 0) {
		return DT_BOOL;
	}

	if (strcmp(token, "DT_FLOAT") == 0) {
		return DT_FLOAT;
	}
	return -1;
}

RC initPageHeader(RM_TableData *rel, Page_Header *pageHeader, int pageId) {
	Table_Header *tableHeader = (Table_Header *)rel->mgmtData;

	pageHeader->pageId = pageId;
	pageHeader->isFull = false;
	pageHeader->freeSlot = 0;
	pageHeader->recordCount = 0;
	pageHeader->recordCapacity = tableHeader->recordsPerPage;

	return RC_OK;

}
