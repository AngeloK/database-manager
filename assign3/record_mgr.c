#include <stdlib.h>
#include <string.h>
#include "record_mgr.h"
#include "tables.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include "dberror.h"

#define EXTRA_SPACE = 100;

// Returns a string of table header from struct
char *structToTableHeader(RM_TableData *tb) {
    char *i = "\0";
    return i;
}

// Returns a string of schema description
char *schemaToString(Schema *schema) {
    char *schemaString = malloc(sizeof(char)*5);
    sprintf(schemaString, "%d|", schema->numAttr);
    if(!schemaString) {
        printf("MEMORY ERROR: Could not allocate to schema string\n");
        exit(1);
    } 
    int i = 0;
    
    // Creates '|' seperated string of attribute description
    // Assumes a max string length of 100
    int MAX_SCHEMA_LEN = 100;
    char *attribute; // Temp string for each attribute
    for(i = 0; i < schema->numAttr; i++) {
        attribute = malloc(sizeof(char)*MAX_SCHEMA_LEN);
        if(!attribute) {
            printf("MEMORY ERROR: Could not allocate to attribute string\n");
            exit(1);
        }
        snprintf(attribute, MAX_SCHEMA_LEN, "%s|%d|%d|",
                schema->attrNames[i], schema->dataTypes[i], schema->typeLength[i]);

        // Ensure memory for final string and concatinate
        schemaString = realloc(schemaString, 
                            sizeof(char)*(strlen(schemaString) + strlen(attribute) + 1));
        if (!schemaString) {
            printf("MEMORY ERROR: Could not re-allocate to schema string\n");
            exit(1);
        }
        strcat(schemaString, attribute);
        free(attribute);
    }


    // Creates a comma seperated string of keyDescs 
    // Assumes integers of max 5 digits
    int MAX_DIGITS = 5;
    char *key; // Temp String for each key
    for (i = 0; i < schema->keySize; i++) {
        key = malloc(sizeof(char)*MAX_DIGITS);
        if (i != schema->keySize-1) {
            snprintf(key, MAX_DIGITS, "%d,", schema->keyAttrs[i]);
        } else {
            snprintf(key, MAX_DIGITS, "%d\n", schema->keyAttrs[i]);
        }

        // Ensure memory for final string and concatinate
        schemaString = realloc(schemaString, sizeof(char)*
                                    (strlen(schemaString) + strlen(key) + 1));
        if(!schemaString) {
            printf("MEMORY ERROR: Could not re-allocate to schema string\n");
            exit(1);
        }
        strcat(schemaString, key);
        free(key);
    }

    return schemaString;
}

void printTableData(RM_TableData *rel) {
    printf("NAME:%s\n", rel->name);
   
    TableMgmt *tm = (TableMgmt *) rel->mgmtData;
    printf("Record Count:%d\n", tm->record_count);
    printf("Page Count:%d\n", tm->page_count);
    printf("Records Count Page:%d\n", tm->records_per_page);
    printf("Reusable Head:%d\n", tm->reusable_head);
    printf("Reusable Tail:%d\n", tm->reusable_tail);

    Schema *s = rel->schema;
    printf("Attributes:%d\n", s->numAttr);
    int i = 0;
    for (i = 0; i < s->numAttr; i++) {
        printf("%s %d %d\n", s->attrNames[i], s->dataTypes[i], s->typeLength[i]);
    }

    for (i = 0; i < s->keySize; i++) {
        printf("(");
        if (i != s->keySize-1) {
            printf("%d,", s->keyAttrs[i]);
        } else {
            printf("%d)", s->keyAttrs[i]);
        }
    }
}

/*Returns Just RC_OK for now. We need to change in future if needed*/ 
RC initRecordManager(void *mgmtData)  
{
   return RC_OK;
}

RC shutdownRecordManager() 
{
   return RC_OK;
}


/*
 * table_name|record_count|page_count|page_capacity|records_per_page|resuable_head|resuable_tail|<schema>
 * <schema>: attrName|dataType|typeLength|...|key1, key2...\n
 *
 */
RC createTable(char *name, Schema *schema) {

    // Create a pageFile
    if( createPageFile(name) != RC_OK) {
        printf("ERROR: Failed to create a page file!");
        exit(1);
    }

    // Create a new buffer pool
    BM_BufferPool *bm = malloc(sizeof(BM_BufferPool));

    // Intial Page size and NULL pointer to StratData
    if(initBufferPool (bm, name, 1000, RS_LRU, NULL) != RC_OK) {
        printf("ERROR: Failed to create buffer pool");
        exit(1);
    }
    
    // Create a '|' seperated description of the table
    // Assume a max header of size of 1000 characters
    char *tableData = malloc(sizeof(char)*1000);
    if (!tableData) {
        printf("ERROR: Memory allocation for table data failed");
        exit(1);
    }
    sprintf(tableData, "%s|0|0|%d|-1|-1|", 
            name, (int)(PAGE_SIZE/sizeof(Schema)));

    char *schemaData = schemaToString(schema);
    
    // Allocate sufficient memory to tableData and concatinate
    tableData = realloc(tableData, sizeof(char)*
                                (strlen(tableData) + strlen(schemaData)));
    strcat(tableData, schemaData);
    
    BM_PageHandle *h = MAKE_PAGE_HANDLE();
    initBufferPool(bm, name, 1, RS_LRU, NULL);
    pinPage(bm, h, 0);
    h->data = tableData;
    markDirty(bm, h);
    unpinPage(bm, h);
    shutdownBufferPool(bm);

    free(h);
    free(tableData);
    free(bm);

    return RC_OK;
}

RC openTable(RM_TableData *rel, char *name) {
    
    if (!rel) {
        printf("MEMORY ERROR: Table data not initialized\n");
        exit(1);
    }

    BM_PageHandle *h = MAKE_PAGE_HANDLE();
    BM_BufferPool *bm = MAKE_POOL();

    initBufferPool(bm, name, 1, RS_LRU, NULL);
    pinPage(bm, h, 0);

    char *tableInfo = h->data;
    const char* delim = "|";

    char *token = strtok(tableInfo, delim);
    if (!token) {
        printf("FILE ERROR: Table not created\n");
        exit(1);
    }

    rel->name = token;
    
    TableMgmt *tm = malloc(sizeof(TableMgmt));
    tm->record_count = atoi(strtok(NULL, delim));
    tm->page_count = atoi(strtok(NULL, delim));
    tm->bm = bm;
    tm->records_per_page = atoi(strtok(NULL, delim));
    tm->reusable_head = atoi(strtok(NULL, delim));
    tm->reusable_tail = atoi(strtok(NULL, delim));

    rel->mgmtData = tm;

    Schema *schema = malloc(sizeof(Schema));
    schema->numAttr = atoi(strtok(NULL, delim));
    schema->attrNames = malloc(sizeof(char*)*schema->numAttr);
    schema->dataTypes = malloc(sizeof(DataType)*schema->numAttr);
    schema->typeLength = malloc(sizeof(int)*schema->numAttr);
    schema->keyAttrs = malloc(0);
    schema->keySize = 0;
     
    int i = 0;
    for (i = 0; i < schema->numAttr; i++) {
        schema->attrNames[i] = strtok(NULL, delim);
        schema->dataTypes[i] = atoi(strtok(NULL, delim));
        schema->typeLength[i] = atoi(strtok(NULL, delim));
    }
    
    char *keyDelim = ",";
    token = strtok(NULL, keyDelim);
    
    while(token) {
        schema->keySize += 1;
        schema->keyAttrs = realloc(schema->keyAttrs, sizeof(int)*schema->keySize);
        schema->keyAttrs[schema->keySize-1] = atoi(token);
        token = strtok(NULL, keyDelim);
    }

    rel->schema = schema;

    printTableData(rel);

    return RC_OK;
}


/*
 * table_name|record_count|page_count|page_capacity|records_per_page|resuable_head|resuable_tail|<schema>
 * <schema>: attrName|dataType|typeLength|...|key1, key2...\n
 *
 */
RC closeTable (RM_TableData *rel) {
    TableMgmt *tm = (TableMgmt *) rel->mgmtData;
    Schema *sm = rel-> schema;
    BM_BufferPool *bm = tm->bm;

    char *tableData = malloc(sizeof(char)*1000);
    if (!tableData) {
        printf("ERROR: Memory allocation for table data failed");
        exit(1);
    }

    sprintf(tableData, "%s|%d|%d|%d|%d|%d|", 
            rel->name, 
            tm->record_count, 
            tm->page_count, 
            (int)(PAGE_SIZE/sizeof(Schema)), 
            tm->reusable_head,
            tm->reusable_tail);
    char *schemaData = schemaToString(sm);
    
    // Allocate sufficient memory to tableData and concatinate
    tableData = realloc(tableData, sizeof(char)*
                                (strlen(tableData) + strlen(schemaData)));
    strcat(tableData, schemaData);
   
    BM_PageHandle *h = MAKE_PAGE_HANDLE(); 
    pinPage(bm, h, 0);
    h->data = tableData;
    markDirty(bm, h);
    unpinPage(bm, h);

    forceFlushPool(bm);

    if (shutdownBufferPool(bm) != RC_OK) {
        printf("BUFFER ERROR: Could not shutdown buffer pool\n");
    }
    
    int i = 0;
    for (i = 0; i < sm->numAttr; i++) {
        free(sm->attrNames[i]);
    }
    free(sm->attrNames);
    free(sm->dataTypes);
    free(sm->typeLength);
    free(sm->keyAttrs);
    free(sm);

    free(tm);
    
    free(rel->name);

    free(rel);

    return RC_OK;
}

RC deleteTable (char *name) {
   destroyPageFile(name); 
}
/* Dealing with Schemas
 *
 *
 *
 */
int getRecordSize(Schema *schema)
{
    int index = 0;
    int size = 0;
    while(index<schema->numAttr){
        if(schema->dataTypes[index] == DT_INT){
            size = size + sizeof(int);
        }
        else if(schema->dataTypes[index] == DT_FLOAT){
            size = size + sizeof(float);
        }
        else if(schema->dataTypes[index] == DT_BOOL){
            size = size + sizeof(float);
        }
        else if(schema->dataTypes[index] == DT_STRING){
            size = size + sizeof(int) + schema->typeLength[index];
        }
        index++;
    }
    return size;
}

Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys)
{
    Schema *schema = (Schema *) malloc(sizeof(Schema));
    schema->numAttr = numAttr;
    schema->keySize = keySize;
    schema->typeLength = typeLength;
    schema->attrNames = attrNames;
    schema->dataTypes = dataTypes;
    schema->keyAttrs = keys;    
    
    return schema;
}

RC freeSchema(Schema *schema)
{
    free(schema->attrNames);
    free(schema->dataTypes);
    free(schema->typeLength);
    free(schema->keyAttrs);
    free(schema);
    schema->attrNames = NULL;
    schema->dataTypes = NULL;
    schema->typeLength = NULL;
    schema->keyAttrs = NULL;
    schema=NULL;
    return RC_OK;
}
/* Dealing with Records
 *
 *
 *
 */
RC createRecord(Record **record, Schema *schema)
{
    *record = (Record *)malloc(sizeof(Record));
    (*record)->data = (char *)malloc(PAGE_SIZE);
    memset((*record)->data,'\0',PAGE_SIZE);
    return RC_OK;
}

RC freeRecord(Record *record)
{
    free(record->data);
    record->data = NULL;
    free(record);
    record = NULL;
    return RC_OK;
}
/* Dealing with Attributes
 *
 *
 *
 */
/*RC getAttr(Record *record, Schema *schema, int attrNum, Value **value){
    
    int position = 0;
    int index = 0;
    Value *val;
    int length;
    while(index<attrNum){
        if(schema->dataTypes[index]==DT_INT || schema->dataTypes[index]==DT_BOOL){
            position = position + sizeof(int);
        }
        else if (schema->dataTypes[index]==DT_FLOAT){
            position = position + sizeof(float);
        }
        else if (schema->dataTypes[index]==DT_STRING){
            position = position + sizeof(int) + schema->typeLength[index];
        }
        index++;
    }
    
    if(schema->dataTypes[attrNum]==DT_INT){
        int integer;
        memcpy(&integer, record->data + position, sizeof(int));
        MAKE_VALUE(val, DT_INT, integer);
    }
    else if(schema->dataTypes[attrNum]==DT_BOOL){
        int integer;
        memcpy(&integer, record->data + position, sizeof(int));
        MAKE_VALUE(val, DT_BOOL, integer);
    }
    else if(schema->dataTypes[attrNum]==DT_FLOAT){
        float f;
        memcpy(&f, record->data + position, sizeof(float));
        MAKE_VALUE(val, DT_FLOAT, f);
    }
    else if(schema->dataTypes[attrNum]==DT_STRING){
        length = 0;
        char c = (char *) malloc(sizeof(char) * schema->typeLength[attrNum]);
        memcpy(&length,record->data + position, sizeof(int));
        memcpy(c, record -> data + position + sizeof(int), length);
        c[length] = '\0';
        MAKE_STRING_VALUE(val, c);
        free(c);
    }
    
    *value = val;
    return RC_OK;
    
}

RC setAttr(Record *record, Schema *schema, int attrNum, Value *value) {
    int position = 0;
    int index = 0;
    
    while(index<attrNum){
        if(schema->dataTypes[index]==DT_INT || schema->dataTypes[index]==DT_BOOL){
            position = position + sizeof(int);
        }
        else if (schema->dataTypes[index]==DT_FLOAT){
            position = position + sizeof(float);
        }
        else if (schema->dataTypes[index]==DT_STRING){
            position = position + sizeof(int) + schema->typeLength[index];
        }
        index++;
    }
    
    if(schema->dataTypes[attrNum]==DT_INT){
        int v = value->v.intV;
        memcpy(record->data + position, &v, sizeof(int));
    }
    else if(schema->dataTypes[attrNum]==DT_BOOL){
        int v = (int)(value->v.boolV);
        memcpy(record->data + position, &v, sizeof(int));
    }
    else if(schema->dataTypes[attrNum]==DT_FLOAT){
        float f = value->v.floatV;
        memcpy(record->data + position, &f, sizeof(float));
    }
    else if(schema->dataTypes[attrNum]==DT_STRING){
        int length = strlen(value->v.stringV);
        char c = (char *) malloc(sizeof(char) * schema->typeLength[attrNum]);
        memcpy(record->data + position,&length, sizeof(int));
        memcpy(record -> data + position + sizeof(int),value->v.stringV, length);
    }
    
    return RC_OK;

    
}*/


