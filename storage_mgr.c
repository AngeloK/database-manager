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

	int mode = S_IRUSR | S_IWUSR;


	int fd = creat(fileName, mode);

	if (fd < 0) {
		printf("file not found.\n");
		return RC_FILE_NOT_FOUND;
	}

	char data[PAGE_SIZE];

	memset(data,'\0',sizeof(data));

	if (write(fd, data, PAGE_SIZE) != PAGE_SIZE) {
		printf("Error writing to file %s\n", fileName);
		return RC_WRITE_FAILED;
	};
	close(fd);
	return RC_OK;

};

RC openPageFile (char *fileName, SM_FileHandle *fHandle) {

	int flag = O_RDONLY;
	int fd = open(fileName, flag);

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

/* reading blocks from disc */
RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
	if (pageNum < fHandle->totalNumPages) {
		return RC_READ_NON_EXISTING_PAGE;
	}	

	


}
int getBlockPos (SM_FileHandle *fHandle);
RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage);
RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage);
RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage);
RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage);
RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage);

/* writing blocks to a page file */
RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage);
RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage);
RC appendEmptyBlock (SM_FileHandle *fHandle);
RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle);




int main(int argc, char *argv[]){
	SM_FileHandle *fHandle = (SM_FileHandle *) malloc(sizeof(SM_FileHandle *));
		
	int c = createPageFile("helloworld.txt");
	openPageFile("helloworld.txt", fHandle);

	printf("%d\n", fHandle->curPagePos);
	printf("%d\n", fHandle->totalNumPages);
	printf("%p\n", fHandle->mgmtInfo);

	int i = closePageFile(fHandle);
	printf("%d\n", i);

	/*destroyPageFile("helloworld.txt");*/
	return 0;
}
