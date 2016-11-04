#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include "table_mgr.h"


RC initTableManager(Table_Header *manager, Schema *schema) {
	Table_Header *tm = (Table_Header *)malloc(sizeof(Table_Header));
	return RC_OK;
}
