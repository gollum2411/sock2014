CC=g++
CFLAGS?=-I. -Wall -Werror

INCLUDES=-Iinclude -Iutils

docdir=doc/
doxyfile = ./sock2014.dox

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

all : $(subprojs) doc
common : $(common_objs)
doc : | $(docdir)
$(objdir)%.o : $(srcdir)%.cpp | $(objdir)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $^

$(subprojs) : $(common_objs)
	$(MAKE) -C $@

$(objdir) :
	@mkdir -p obj

doc :
	@echo Generating doxygen documentation
	doxygen $(doxyfile) 2>&1 > /dev/null

$(docdir) :
	mkdir -p $@

clean : cleandoc
	@for d in ${subprojs}; do make -C $$d clean ; done
	rm -rf $(objdir)

cleandoc :
	rm -rf $(docdir)


.PHONY : all $(subprojs) $(objdir) common
