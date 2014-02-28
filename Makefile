CC=g++
CFLAGS?=-I. -Wall -Werror

INCLUDES=-Iinclude -Iutils

objdir=obj/
common_objs_list = gollum2411.o
common_objs=$(addprefix $(objdir),$(common_objs_list))

srcdir=src/

CFLAGS+=$(INCLUDES)

ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -g
endif

subprojs = utils timeserver filetransfer\
	   httpserver

all : $(subprojs)
common : $(common_objs)

$(objdir)%.o : $(srcdir)%.cpp | $(objdir)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $^

$(subprojs) : $(common_objs)
	$(MAKE) -C $@

$(objdir) :
	@mkdir -p obj

clean :
	@for d in ${subprojs}; do make -C $$d clean ; done
	rm -rf $(objdir)


.PHONY : all $(subprojs) $(objdir) common
