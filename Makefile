objects = storage_mgr.c dberror.c test_assign1_1.c 
exe_obj = test_assign

test : $(objects) 
	gcc -o $(exe_obj) $(objects) 
	./$(exe_obj)

clean :
	rm $(exe_obj)