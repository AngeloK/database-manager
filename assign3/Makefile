CC=gcc
CFLAGS=-Wall -g

all: testSerial clean

testSerial: main.o record_mgr.o buffer_mgr.o storage_mgr.o buffer_pool.o buffer_mgr_stat.o dberror.o
	$(CC) $(CFLAGS) main.o record_mgr.o buffer_mgr.o storage_mgr.o buffer_mgr_stat.o buffer_pool.o dberror.o -o testSerial

#main.o: main.c record_mgr.o tables.h
#	$(CC) $(CFLAGS) main.c
#
#record_mgr.o: record_mgr.h tables.h buffer_mgr.o dberror.c
#	$(CC) $(CFLAGS) record_mgr.c
#
#buffer_mgr.o: buffer_mgr.c buffer_mgr.h storage_mgr.o
#	$(CC) $(CFLAGS) buffer_mgr.c
#
#storage_mgr.o: storage_mgr.c storage_mgr.h
#	$(CC) $(CFLAGS) storage_mgr.c
#
clean:
	rm -rf *.o
