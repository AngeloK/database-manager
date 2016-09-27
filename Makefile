objects = storage_mgr.c dberror.c test_assign1_1.c 
exe_obj = test_assign 
tem_file = test_pagefile.bin

test : $(objects) 
	gcc -o $(exe_obj) $(objects) 
	./$(exe_obj)

clean :
	rm $(exe_obj) $(test_pagefile)
