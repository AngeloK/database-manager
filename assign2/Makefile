objects = buffer_mgr.c storage_mgr.c dberror.c buffer_controller.c
exe_obj = main.c
# test_1_obj = test_assign1_1.c
# test_2_obj = test_assign1_2.c
# exe_obj = test_assign 
output = main
# tem_file = test_pagefile.bin

# all : test1 test2
# 
# test1 : $(objects) $(test_1_obj)
# 	gcc -o $(exe_obj) $(objects) $(test_1_obj) 
# 	./$(exe_obj)
# 
# test2 : $(objects) $(test_2_obj)
# 	gcc -o $(exe_obj) $(objects) $(test_2_obj)
# 	./$(exe_obj)


all : run

run : $(objects) $(exe_obj) 
	gcc -o $(output) $(objects) $(exe_obj)
	./$(output)

# clean :
# 	rm $(exe_obj) $(test_pagefile)
