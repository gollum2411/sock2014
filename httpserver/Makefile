CC=g++
CFLAGS?=-I. -Wall -Werror

INCLUDES=-I../utils/ -I../include

CFLAGS+=$(INCLUDES)

bindir=bin/
_bin_name=httpserver
bin=$(addprefix $(bindir),$(_bin_name))
$(bin) : | $(bindir)

objdir=obj/
objs_list=httpserver.o
objs=$(addprefix $(objdir),$(objs_list))
$(objs) : | $(objdir)

utils_dir=../utils/
utils_objs_dir=$(utils_dir)obj/
utils_objs_list=utils.o
utils_objs=$(addprefix $(utils_objs_dir),$(utils_objs_list))

common_objs_dir=../obj/
common_objs_list=gollum2411.o
common_objs=$(addprefix $(common_objs_dir),$(common_objs_list))
common_src_dir=../src/

ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -g
endif

all : $(bin)

$(objdir) :
	@echo Creating $@
	@mkdir -p $@

$(bindir) :
	@echo Creating $@
	@mkdir -p $@

$(bin) : $(objs) $(utils_objs) $(common_objs)
	$(CC) $(CFLAGS) -o $@ $^
	@echo Done

$(objdir)%.o : %.cpp
	@echo Compiling objects from $^
	$(CC) $(CFLAGS) -c -o $@ $^

$(utils_objs_dir)%.o : $(utils_dir)%.cpp
	@echo Compiling utils
	make -C ../utils

$(common_objs_dir)%.o : $(common_src_dir)%.cpp
	@echo Compiling common
	make -C ../ common

clean :
	rm -rf $(bindir) $(objdir)

.PHONY : clean all
