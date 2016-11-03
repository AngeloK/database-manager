end: recordManager clean

recordManager:test_assign3_1.o dberror.o storage_mgr.o buffer_mgr.o buffer_mgr_stat.o
	gcc -g test_assign3_1.o dberror.o storage_mgr.o buffer_mgr.o buffer_mgr_stat.o -o recordManager


test_assign2_1.o :test_assign3_1.c test_helper.h dberror.h storage_mgr.h buffer_mgr.h buffer_mgr_stat.h record_mgr.h
	gcc -c test_assign3_1.c

dberror.o:dberror.c dberror.h
	gcc -c dberror.c

storage_mgr.o:storage_mgr.c storage_mgr.h dberror.h
	gcc -c storage_mgr.c
				
# record_mgr.o:record_mgr.c record_mgr.h
# 	gcc -c record_mgr.c

buffer_mgr.o:buffer_mgr.c buffer_mgr.h
	gcc -c buffer_mgr.c

buffer_mgr_stat.o:buffer_mgr_stat.c buffer_mgr_stat.h
	gcc -c buffer_mgr_stat.c

clean:
	-rm -rf *.o 
