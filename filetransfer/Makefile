CC?=g++
CFLAGS?=-I. -Wall -Werror

INCLUDES=-I../utils/

CFLAGS+=$(INCLUDES)

bindir=bin/
_bin_name=filetransfer
bin=$(addprefix $(bindir),$(_bin_name))
$(bin) : | $(bindir)

objdir=obj/
objs_list=filetransfer.o server.o client.o
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
	@mkdir -p $@

$(bindir) :
	@mkdir -p $@

$(bin) : $(objs) $(utils_objs)
	$(CC) $(CFLAGS) -o $@ $^
	@echo Done

$(objdir)%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(utils_objs_dir)%.o : $(utils_dir)%.c
	make -C ../utils

clean :
	rm -rf $(bindir) $(objdir)

.PHONY : clean all
