objects = buffer_mgr.c storage_mgr.c dberror.c buffer_pool.c buffer_mgr_stat.c
test_1_obj = test_assign2_1.c
exe_obj = test_assign 
tem_file = test_pagefile.bin

all : test1

test1 : $(objects) $(test_1_obj)
	gcc -o $(exe_obj) $(objects) $(test_1_obj) 
	./$(exe_obj)

clean :
	rm $(exe_obj) $(test_pagefile)
