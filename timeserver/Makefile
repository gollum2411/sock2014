CC?=gcc
CFLAGS?=-I. -Wall -Werror

INCLUDES=-I../utils/

CFLAGS+=$(INCLUDES)

bindir=bin/
_bin_name=timeserver
bin=$(addprefix $(bindir),$(_bin_name))
$(bin) : | $(bindir)

objdir=obj/
objs_list=timeserver.o
objs=$(addprefix $(objdir),$(objs_list))
$(objs) : | $(objdir)

utils_dir=../utils/
utils_objs_dir=$(utils_dir)obj/
utils_objs_list=utils.o
utils_objs=$(addprefix $(utils_objs_dir),$(utils_objs_list))

ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -g
endif

all : $(bin)

$(objdir) :
	@echo Creating $@
	mkdir -p $@

$(bindir) :
	@echo Creating $@
	mkdir -p $@

$(bin) : $(objs) $(utils_objs)
	$(CC) $(CFLAGS) -o $@ $^
	@echo Done

$(objdir)%.o : %.c
	@echo Compiling objects from $^
	$(CC) $(CFLAGS) -c -o $@ $^

$(utils_objs_dir)%.o : $(utils_dir)%.c
	@echo Compiling utils
	make -C ../utils

clean :
	rm -rf $(bindir) $(objdir)

.PHONY : clean all
