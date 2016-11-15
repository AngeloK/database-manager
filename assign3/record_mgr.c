#include <stdlib.h>
#include <string.h>
#include "record_mgr.h"
#include "tables.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include "list.h"

#define EXTRA_SPACE = 100;

// Returns a string of table header from struct
char *structToTableHeader(RM_TableData *tb) {

}

// Returns a string of schema description
char *schemaToString(Schema *schema) {
    char *attributes = malloc(sizeof(char));
    char *schema = malloc(sizeof(char)*100);
    
    int i = 0;
    for(i = 0; i < schema->numAttr: i++) {
        // Assumes a max string length of 100
        sprintf(schema, "%s|%d|%d\n",
                schema->attrNames[i], schema->dataTypes[i], schema->typeLength[i]);
        attributes = strcat(attributes, schema);
    }

    return attributes;
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

RC createTable(char *name, Schema *schema) {

    // Create a pageFile
    if( createPageFile(name) != RC_OK) {
        print("ERROR: Failed to create a page file!");
    }

    // Create a new buffer pool
    BM_BufferPool *bm = malloc(sizeof(BM_BufferPool));

    // Intial Page size and NULL pointer to StratData
    if(initBufferPool (bm, name, 1000, RS_LRU, NULL) != RC_OK) {
        printf("ERROR: Failed to create buffer pool");
    }
    
    // Assume a max header of size of 1000 characters
    char *tableData = malloc(sizeof(char*1000));
    sprintf(tableHead, "%s&0&0&%d&%d&-1&-1&%d\n", 
            name, PAGE_SIZE, (int)PAGE_SIZE/sizeof(Schema), schema->numAttr);

    char *schemaData = schemaToString(schema);

    char *tableHead = strcat(tableData, schemaData);
    free(tableData);
    free(schemaData);
    
    char *key = malloc(sizeof(char));
    int i = 0;
    for (i = 0; i < schema->keySize; i++) {
        if (i < schema->keySize) {
            sprintf(key, "%d,", schema->keyDesc[i]);
            tableHead = strcat(

    }
    char *keyDesc = malloc(sizeof(char)*schema->keySize*3);
    

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
    while(i<schema->numAttr){
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


