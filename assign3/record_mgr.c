#include <stdlib.h>
#include <string.h>
#include "record_mgr.h"
#include "tables.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include "list.h"

/*Returns Just RC_OK for now. We need to change in future if needed*/ 
RC initRecordManager(void *mgmtData)  
{
   return RC_OK;
}

RC shutdownRecordManager() 
{
   return RC_OK;
}
/* Dealing with schemas
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

