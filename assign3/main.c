#include <stdio.h>
#include <stdlib.h>
#include "record_mgr.h"
#include "tables.h"

int main (int argc,  char* argv[]) {
    Schema *s = malloc(sizeof(Schema));
    s->numAttr = 3;
    char *names[] = {"Attr1", "Attr2", "Attr3"}; 
    s->attrNames = names; 
    DataType data[] = {DT_INT, DT_INT, DT_INT};
    s->dataTypes = data;
    int len[] = {0, 2, 0};
    s->typeLength = len;
    int keys[] = {0, 1};
    s->keyAttrs = keys;
    s->keySize = 2;

    createTable("newTable.dat", s);

    return 1;
}
