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
#include "list.h"

// table and manager
RC initRecordManager (void *mgmtData) {

  return RC_OK;
}
RC shutdownRecordManager () {
  //free memeory.
	return RC_OK;
}
/**
 * create a table file and write table header and schema into file.
 * @param  name   table file name
 * @param  schema schema of record manager.
 * @return       	RC_OK;
 */
RC createTable (char *name, Schema *schema) {
  RM_TableData *table = (RM_TableData *) malloc(sizeof(RM_TableData));
	Page_Header *pageHeader = (Page_Header *)malloc(sizeof(Page_Header));

  // assign name and schema to RM_TableData.
	table->name = name;
	table->schema = schema;

  // initialize table header.
	Table_Header *tableHeader = (Table_Header *)malloc(sizeof(Table_Header));
	initTableManager(tableHeader, schema);

  // assign table header to mgmtData.
	table->mgmtData = tableHeader;

  // insert table header and page header of the first page
  // into page file via buffer manager.
  BM_BufferPool *bm = MAKE_POOL();
  BM_PageHandle *h = MAKE_PAGE_HANDLE();
  SM_FileHandle fh;

  createPageFile(name);

  initBufferPool(bm, name, 5, RS_FIFO, NULL);

	pinPage(bm, h, 0);
	h->data = generateTableInfo(table);
	markDirty(bm, h);
	unpinPage(bm, h);


	//write tombstone list into the second page of table file.

	pinPage(bm, h, 1);

	initPageHeader(table, pageHeader, 1);
	h->data = generatePageHeader(table, pageHeader);
	markDirty(bm, h);
	unpinPage(bm, h);

	shutdownBufferPool(bm);

  // free memeory.
	free(h);
  closePageFile(&fh);
	free(table);
	free(pageHeader);
	free(tableHeader);
  return RC_OK;

  // here just create a table with a name.
}
RC openTable (RM_TableData *rel, char *name) {
  // Open a table via table name.
  rel->name = name;
	SM_FileHandle fh;
	SM_PageHandle ph;
	ph = (SM_PageHandle) malloc(PAGE_SIZE);

	openPageFile(name, &fh);
  // read the first page of page file.
	readBlock(0, &fh, ph);

  // initialize schema and table header by deserialize information stored in
  // the first page.
	rel->schema = (Schema *)malloc(sizeof(Schema));
	parseTableHeader(rel, ph);

	Table_Header *tableHeader = (Table_Header *)rel->mgmtData;
	closePageFile(&fh);

	return RC_OK;
}


/**
 * close table and free memeory.
 * @param  rel a RM_TableData variable
 * @return     RC_OK
 */
RC closeTable (RM_TableData *rel) {
	Table_Header *tableHeader = (Table_Header *)rel->mgmtData;

	ListNode *head = tableHeader->tombstone->head;

	RID *id;

	while(head->next) {
		id = (RID *)head->value;
		printf("is page is %d\n", id->page);
		printf("is slot is %d\n", id->slot);
		printf("====");
		head = head->next;
	}

  // close table and free memeory.
  freeSchema(rel->schema);
  free(rel->mgmtData);

  return RC_OK;

}
/**
 * delete page file.
 * @param  name table name.
 * @return      RC_OK
 */
RC deleteTable (char *name) {
  destroyPageFile(name);
  return RC_OK;
}

/**
 * get total count of records stored in record manager.
 * @param  rel RM_TableData
 * @return     RC_OK
 */
int getNumTuples (RM_TableData *rel) {
	Table_Header *tableHeader = (Table_Header *)rel->mgmtData;
  return tableHeader->totalRecordCount;
}

/**
 * Insert record into record manager.
 * @param  rel    RM_TableData
 * @param  record the record needs to be inserted.
 * @return        RC_OK
 */
RC insertRecord (RM_TableData *rel, Record *record) {
	Table_Header *tableHeader = (Table_Header *)rel->mgmtData;

  // 'freePointer' is used to find the first empty slot.
	RID *freePointer = tableHeader->freePointer;
	RID *rid = (RID  *)malloc(sizeof(RID));


	// TODO check primiary key.

	SM_FileHandle fh;
	SM_PageHandle ph;
	ph = (SM_PageHandle) malloc(PAGE_SIZE);
	int i;
	int offset = 50 + (freePointer->slot) * (schemaLength(rel->schema));

  // serialize the record with separator defined as "&".
	openPageFile(rel->name, &fh);
	readBlock(freePointer->page, &fh, ph);
	Value *value;
  VarString *result;
  MAKE_VARSTRING(result);


	// TODO select rid in tombstone list first instead of find the first
	// empty slot.


	for (i = 0; i < rel->schema->numAttr; i++) {
		getAttr(record, rel->schema, i, &value);

		APPEND(result, "%s&", serializeValue(value));
	}
	freeVal(value);

  // find the offset of the record and copy it to that place.
	memcpy(ph+offset, result->buf, strlen(result->buf));

	// after a new record has been added, we increase the recordCount by 1 and
	// update the page header;
	Page_Header *updatedHeader = (Page_Header *)malloc(sizeof(Page_Header));
	char *header = (char *)malloc(sizeof(char) * 50);
	//
	memcpy(header, ph, 50);
	deserializePageHeader(header, updatedHeader);

	updatedHeader->recordCount++;

	if(updatedHeader->recordCount > updatedHeader->recordCapacity - 1) {
    // assign isFull to true if recordCount reaches the recordCapacity.
		updatedHeader->isFull = 1;
	}

	char *updatedHeaderStr = generatePageHeader(rel, updatedHeader);

	memcpy(ph, updatedHeaderStr, strlen(updatedHeaderStr));
	writeBlock(freePointer->page, &fh, ph);

  // assing rid (current position) to record.
	rid->page = freePointer->page;
	rid->slot = freePointer->slot;

  // move freePointer to next position, if it reaches the maximum record count,
  // a new page(with page header) is added to page file.
	freePointer->slot++;
	if (freePointer->slot > tableHeader->recordsPerPage - 1) {
		freePointer->slot = 0;
		freePointer->page++;
		Page_Header *pageHeader = (Page_Header *)malloc(sizeof(Page_Header));

		initPageHeader(rel, pageHeader, freePointer->page);
		char *s;
		s = generatePageHeader(rel, pageHeader);
		ensureCapacity(freePointer->page, &fh);
		writeBlock(freePointer->page, &fh, s);
		free(pageHeader);
		free(s);

	}

  // update table header.
	tableHeader->pageCount = freePointer->page;
	tableHeader->freePointer = freePointer;
	tableHeader->totalRecordCount++;
	readBlock(0, &fh, ph);

	char *tableHeaderStr = generateTableInfo(rel);
	memcpy(ph, tableHeaderStr, strlen(tableHeaderStr));
	writeBlock(0, &fh, ph);

	record->id = *rid;

	closePageFile(&fh);
	// free(rid);
	// free(updatedHeader);
	// free(updatedHeaderStr);
	// free(header);
	// free(tableHeaderStr);
	// free(ph);
  return RC_OK;
}

/**
 * delete a record
 * @param  rel RM_TableData
 * @param  id  the id of the record needs to be deleted.
 * @return     RC_OK
 */
RC deleteRecord (RM_TableData *rel, RID id) {
  // define r;
  // getRecord(rel, id, r);
  // mark r as deleted.
  Table_Header *tableHeader = (Table_Header *)rel->mgmtData;
	RID *tstone_id = (RID *)malloc(sizeof(RID));
	tstone_id->page = id.page;
	tstone_id->slot = id.slot;
	if (insert(tableHeader->tombstone, tstone_id) == 0 ) {
		// TODO update header and tombstone.
		// update relative page header.
		//
		// update tombstone stored in table file.
		return RC_OK;
	}
	return -1;
}
RC updateRecord (RM_TableData *rel, Record *record) {
  // define a new r;
	Record *r = (Record *)malloc(sizeof(Record));

	getRecord(rel, record->id, r);

	Value *value;
	VarString *result;
	MAKE_VARSTRING(result);

	int i;

	for (i = 0; i < rel->schema->numAttr; i++) {
		getAttr(record, rel->schema, i, &value);
		setAttr(r, rel->schema, i, value);
		freeVal(value);
	}


	for (i = 0; i < rel->schema->numAttr; i++) {
		getAttr(r, rel->schema, i, &value);
		APPEND(result, "%s&", serializeValue(value));
		freeVal(value);
	}

	SM_FileHandle fh;
	SM_PageHandle ph;
	ph = (SM_PageHandle) malloc(PAGE_SIZE);

	openPageFile(rel->name, &fh);
	int offset = 50 + (record->id.slot) * (schemaLength(rel->schema));
	readBlock(record->id.page, &fh, ph);
	strncpy(ph+offset, result->buf, schemaLength(rel->schema));
	writeBlock(record->id.page, &fh, ph);
	closePageFile(&fh);
	free(ph);
	FREE_VARSTRING(result);
	free(r);

	return RC_OK;
}
RC getRecord(RM_TableData *rel, RID id, Record *record) {

	Table_Header *tableHeader = (Table_Header *)rel->mgmtData;


	int offset = 50 + (id.slot) * (schemaLength(rel->schema));
	SM_PageHandle p = (SM_PageHandle) malloc(PAGE_SIZE);

	SM_FileHandle fh;
	SM_PageHandle ph;
	ph = (SM_PageHandle) malloc(PAGE_SIZE);

	openPageFile(rel->name, &fh);
	readBlock(id.page, &fh, ph);

	char *pageHeaderStr = (char *)malloc(sizeof(char) * 50);
	Page_Header *pageHeader = (Page_Header *)malloc(sizeof(Page_Header));

	memcpy(pageHeaderStr, ph, 50);
	deserializePageHeader(pageHeaderStr, pageHeader);

	if (id.page >tableHeader->pageCount || id.slot > pageHeader->recordCount) {
		return RC_RM_NO_MORE_TUPLES;
	}

	memcpy(p, ph+offset, schemaLength(rel->schema));

	Record *r = deserializeRecord(rel->schema, p, id);
	record->id = r->id;
	record->data = r->data;

	closePageFile(&fh);


  return RC_OK;
}


// scans
RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond) {
	ScanInfo *scanInfo = (ScanInfo *)malloc(sizeof(ScanInfo));
	scanInfo->cond = cond;
	RID startRID;
	startRID.page = 1;
	startRID.slot = 0;
	scanInfo->curRID = startRID;

	scan->mgmtData = (void *)scanInfo;
	scan->rel = rel;

	return RC_OK;
}
RC next (RM_ScanHandle *scan, Record *record) {
	ScanInfo *scanInfo = (ScanInfo *)scan->mgmtData;
	RID currentRID = scanInfo->curRID;
	Value *value;

	SM_FileHandle fh;
	SM_PageHandle ph;
	ph = (SM_PageHandle) malloc(PAGE_SIZE);

	openPageFile(scan->rel->name, &fh);
	readBlock(currentRID.page, &fh, ph);

	char *pageHeaderStr = (char *)malloc(sizeof(char) * 50);
	Page_Header *pageHeader = (Page_Header *)malloc(sizeof(Page_Header));

	memcpy(pageHeaderStr, ph, 50);
	deserializePageHeader(pageHeaderStr, pageHeader);

	if (currentRID.slot > pageHeader->recordCount) {
		return RC_RM_NO_MORE_TUPLES;
	}

	int i;
	for (i = currentRID.slot; i < pageHeader->recordCount; i++) {
		RID fetchRId;
		fetchRId.page = currentRID.page;
		fetchRId.slot = i;
		RC fetch = getRecord(scan->rel, fetchRId, record);

		if (fetch != RC_RM_NO_MORE_TUPLES) {
			evalExpr(record, scan->rel->schema, scanInfo->cond, &value);
			if (value->v.boolV == 1) {
				scanInfo->curRID.slot = fetchRId.slot+1;
				free(ph);
				closePageFile(&fh);
				free(pageHeader);
				free(pageHeaderStr);
				return RC_OK;
			}
		}
	}

	free(ph);
	closePageFile(&fh);
	free(pageHeader);
	free(pageHeaderStr);

	return RC_RM_NO_MORE_TUPLES;
}
RC closeScan (RM_ScanHandle *scan) {
	return RC_OK;
}

// dealing with schemas
int getRecordSize (Schema *schema) {
	int schemaLength= 0;
	int i;
	for (i = 0; i < schema->numAttr; i++) {
		if (schema->dataTypes[i] == DT_STRING) {
		schemaLength+= schema->typeLength[i];
		}
		else {
		schemaLength+= 1;
		}
	}
	return schemaLength;
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
  free(schema->dataTypes);

  free(schema->attrNames);

  free(schema->keyAttrs);
  free(schema->typeLength);

  free(schema);
	return RC_OK;
}


int schemaLength(Schema *schema) {
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
	// int recordLength = schemaLength(schema);
	int recordLength = getRecordSize(schema);

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
			memcpy(&((*value)->v.intV), valueFromRecord, sizeof(int));
			break;
		case DT_STRING:
			(*value)->dt = DT_STRING;
			int stringLength = schema->typeLength[attrNum];
			(*value)->v.stringV = (char *)malloc(sizeof(char) * stringLength + 1);
			// // Allocate space.
			strncpy((*value)->v.stringV, valueFromRecord, stringLength);
			(*value)->v.stringV[stringLength] = '\0';
			break;
		case DT_BOOL:
			(*value)->dt = DT_BOOL;
			memcpy(&((*value)->v.boolV), valueFromRecord, sizeof(bool));
			break;
		case DT_FLOAT:
			(*value)->dt = DT_FLOAT;
			memcpy(&((*value)->v.floatV), valueFromRecord, sizeof(float));
			break;
	}

  return RC_OK;
}

RC setAttr (Record *record, Schema *schema, int attrNum, Value *value){
		int offset;
    attrOffset(schema, attrNum, &offset);
		char *result = record->data+offset;

		switch (value->dt) {
			case DT_INT:
				memcpy(result, &(value->v.intV) ,sizeof(int));
				break;
			case DT_STRING:
				memcpy(result, value->v.stringV, schema->typeLength[attrNum]);
				break;
			case DT_FLOAT:
				memcpy(result, &(value->v.floatV) ,sizeof(float));
				break;
			case DT_BOOL:
				memcpy(result, &(value->v.boolV) ,sizeof(bool));
				break;
		}
		return RC_OK;
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
	// char *timer = (char *)malloc(26);
	// char *r;

	int recordLen = schemaLength(rel->schema);

	APPEND(result, "%s", rel->name);

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
	APPEND(result, "%d", tableHeader->totalRecordCount);
	APPEND_STRING(result, "&");

	// free page.
	APPEND(result, "%d", tableHeader->freePointer->page);
	APPEND_STRING(result, "&");

	APPEND(result, "%d", tableHeader->freePointer->slot);
	APPEND_STRING(result, "&");

	// lastAccessed.
	// currentTime(timer);
	APPEND_STRING(result, "2016-11-13 23:31:24&");
	// APPEND_STRING(result, "&");

	// APPEND_STRING(result, serializeSchema(rel->schema));

	// schema->numAttr.
	APPEND(result, "%d", rel->schema->numAttr);
	// APPEND(result, "%d", 3);
	APPEND_STRING(result, "&");

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
	// free(timer);
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


RC deserializePageHeader(char *str, Page_Header *pageHeader) {

	int vals[4], i;

	i = 0;

	char *token;
	token = strtok(str, "&");
	while(token != NULL) {
		vals[i] = atoi(token);
		token = strtok(NULL, "&");
		i++;
	}
	int *cpVals = (int *)malloc(sizeof(int) * 4);

	memcpy(cpVals, vals, sizeof(int) * 4);

	pageHeader->pageId = cpVals[0];
	pageHeader->isFull = cpVals[1];
	pageHeader->recordCount = cpVals[2];
	pageHeader->recordCapacity = cpVals[3];

	return RC_OK;
}



char *generatePageHeader(RM_TableData *rel, Page_Header *pageHeader) {
	VarString *result;
	MAKE_VARSTRING(result);

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
	APPEND_STRING(result, "&");
	// APPEND(result, "%d", 1000);

	RETURN_STRING(result);
}

RC initTableManager(Table_Header *manager, Schema *schema) {

	int schemaLen = schemaLength(schema);

	manager->tableCapacity = (TOTAL_PAGES - 1) * ((PAGE_SIZE - 50)/schemaLen);
	manager->pageCount = 0;

	char *timer = (char *)malloc(26);
	currentTime(timer);
	manager->lastAccessed = timer;

	manager->recordsPerPage = (PAGE_SIZE - 50)/schemaLen;

	RID * freePointer = (RID *)malloc(sizeof(RID));
	freePointer->page = 1;
	freePointer->slot = 0;
	manager->freePointer = freePointer;

	return RC_OK;
}

RC parseTableHeader(RM_TableData *rel, char *stringHeader) {

	Schema *schema;
	Table_Header *tableHeader = (Table_Header *)malloc(sizeof(Table_Header));

	char *tableAttrs[8];
	char *token;
	token = strtok(stringHeader, "&");
	int i = 0;
	while (token != NULL && i < 8)
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
	// TODO primary key used for primary key check.
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

	RID *freePointer = (RID *)malloc(sizeof(RID));



	tableHeader->tableCapacity = atoi(tableAttrs[1]);
	tableHeader->recordsPerPage = atoi(tableAttrs[2]);
	tableHeader->pageCount = atoi(tableAttrs[3]);
	tableHeader->totalRecordCount = atoi(tableAttrs[4]);
	freePointer->page = atoi(tableAttrs[5]);
	freePointer->slot = atoi(tableAttrs[6]);
	tableHeader->lastAccessed = tableAttrs[7];

  // tableHeader->tombstone = createList();

	tableHeader->freePointer = freePointer;

	rel->mgmtData = tableHeader;



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


Record *deserializeRecord(Schema *schema, char *recordString, RID id) {
	char *token;
	token = strtok(recordString, "&");
	int i = 0;

	char **temp = malloc((schema->numAttr) * sizeof(char *));

	while (token != NULL && i < schema->numAttr)
	{
		temp[i] = malloc(strlen(token));
		temp[i] = token;
		token = strtok (NULL, "&");
		i++;
	}

	Record *record;
	Value *value = (Value *)malloc(sizeof(Value));

	createRecord(&record, schema);

	int offset;

	for (i = 0; i < schema->numAttr; i++) {

		switch (schema->dataTypes[i]) {
			case DT_INT:
				MAKE_VALUE(value, DT_INT, atoi(temp[i]));
				setAttr(record, schema, i, value);
				break;
			case DT_STRING:
				MAKE_STRING_VALUE(value, temp[i]);
				setAttr(record, schema, i, value);
				break;
			case DT_FLOAT:
				MAKE_VALUE(value, DT_FLOAT, atoi(temp[i]));
				setAttr(record, schema, i, value);
				break;
			case DT_BOOL:
				MAKE_VALUE(value, DT_BOOL, strtof(temp[i], NULL));
				setAttr(record, schema, i, value);
				break;
		}
		freeVal(value);
	}
	free(temp);
	record->id = id;

	return record;
}
