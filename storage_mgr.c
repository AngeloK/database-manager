#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h> // for close
#include <errno.h>
#include <sys/stat.h>

#include "storage_mgr.h"
#include "dberror.h"
#include "test_helper.h"

#define TOTAL_PAGES 1000



void initStorageManager (void) {

	SM_FileHandle *fHandle = (SM_FileHandle *) malloc(sizeof(SM_FileHandle *));

};
RC createPageFile (char *fileName) {
	FILE *fp;

	fp = fopen(fileName, "w");

	if (fp == NULL) {
		return RC_FILE_NOT_FOUND;
	}

	char data[PAGE_SIZE];

	memset(data,'\0',sizeof(data));
	fwrite(data, PAGE_SIZE, 1, fp);
	
	fclose(fp);
	return RC_OK;
};

RC openPageFile (char *fileName, SM_FileHandle *fHandle) {
	int fd = open(fileName, O_RDONLY);
	if (fcntl(fd, F_GETFL) < 0 && errno == EBADF) {
		return RC_FILE_NOT_FOUND;
	}

	fHandle->mgmtInfo = (void *)fd;

	struct stat sat;
	fstat(fd, &sat);
	int size = sat.st_size;
	
	fHandle->fileName = fileName;
	fHandle->totalNumPages = size/PAGE_SIZE;
	fHandle->curPagePos = 0;

	return RC_OK;
}

RC closePageFile (SM_FileHandle *fHandle) {
	int fd = fHandle->mgmtInfo;
	if (close(fd)) {
		return RC_OK;
	}
	return RC_FILE_NOT_FOUND;
};
RC destroyPageFile (char *fileName) {
	int r = remove(fileName);
	if (r == 0) {
		return RC_OK;
	}
	return RC_FILE_NOT_FOUND;
}


int main(int argc, char *argv[]){
	SM_FileHandle *fHandle;
	initStorageManager();
		
	createPageFile("helloworld.txt");
	openPageFile("helloworld.txt", fHandle);

	printf("%d\n", fHandle->curPagePos);
	printf("%d\n", fHandle->totalNumPages);
	printf("%p\n", fHandle->mgmtInfo);

	int i = closePageFile(fHandle);
	printf("%d\n", i);

	destroyPageFile("helloworld.txt");
	return 0;
}
