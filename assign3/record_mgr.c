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

	table->name = name;
	table->schema = schema;
	table->mgmtData = initTableManager(schema);

  BM_BufferPool *bm = MAKE_POOL();
  BM_PageHandle *h = MAKE_PAGE_HANDLE();
  SM_FileHandle fh;

  createPageFile(name);

  initBufferPool(bm, name, 5, RS_FIFO, NULL);

	// pinPage(bm, h, 1);
	// add header data into pagefile.
  // sprintf(h->data, "%s-%i", "Page", h->pageNum);
	// markDirty(bm, h);
	// unpinPage(bm, h);

	// pinPage(bm, h, 2);
	// add page header data into the first page;
	// sprintf(->data, "", );
	// markDirty(bm, h);
	// unpinPage(bm, h)

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




  // read from table header, save all necessary info into mgmtData.
  //rel->mgmtData = ?.
  // the data stored in mgmtData should be decided after discussion.

  // mgmtData.status = 0;
  // closed table.
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
  // I think here we have a pointer points to the free space, so we are able to count
  // the pages based on the pointer.
  // return rel->mgmtData->count;
  return 0;
}


RC insertRecord (RM_TableData *rel, Record *record) {
  // inside the record, we have the RID.
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
			// (*value)->v.boolV = transferTo(valueFromRecord, DT_BOOL);
			// (*value)->v.boolV = NULL;
			break;
		case DT_FLOAT:
			(*value)->dt = DT_FLOAT;
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
	char *t = (char *)malloc(19);
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
	char *t1 = (char *)malloc(19);
	char *r;

	// int recordLen = schemaLength(rel->schema);
	int recordLen = 26;

	// APPEND(result, "%s", rel->name);
	APPEND(result, "%s", "Hello_World");

	APPEND_STRING(result, "&");

	// tableCapacity.
	APPEND(result, "%d", 999 * (PAGE_SIZE/recordLen));
	APPEND_STRING(result, "&");

	// pageCapacity.
	APPEND(result, "%d", PAGE_SIZE/recordLen);
	APPEND_STRING(result, "&");

	// pageCount.
	APPEND(result, "%d", 100);
	APPEND_STRING(result, "&");

	// recordCount.
	// APPEND(result, "%d", getNumTuples(rel));
	APPEND(result, "%d", 150);
	APPEND_STRING(result, "&");

	// lastAccessed.
	currentTime(t1);
	APPEND_STRING(result, t1);
	APPEND_STRING(result, "&");

	// APPEND_STRING(result, serializeSchema(rel->schema));


	// schema->numAttr.
	// APPEND_STRING(result, rel->schema->numAttr);
	APPEND(result, "%d", 3);
	APPEND_STRING(result, "&");

	// TODO schema format

  // APPEND(result, "TABLE <%s> with <%i> tuples:\n", rel->name, getNumTuples(rel));
  // APPEND_STRING(result, serializeSchema(rel->schema));

	// puts(RETURN_STRING(result));
	// printf("%s\n", RETURN_STRING(result));
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



int tableLength(RM_TableData *rel) {

}
