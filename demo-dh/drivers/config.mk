cur_dir :=$(dir $(lastword $(MAKEFILE_LIST)))

obj-y	+=$(patsubst $(cur_dir)%.c,$(cur_dir)%.o,$(wildcard $(cur_dir)*.c))
obj-y	+=$(patsubst $(cur_dir)%.cpp,$(cur_dir)%.o,$(wildcard $(cur_dir)*.cpp))
