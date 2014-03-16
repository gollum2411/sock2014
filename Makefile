CC=g++
CFLAGS?=-I. -Wall -Werror -std=c++0x

INCLUDES=-Iinclude -Iutils

ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -g
endif

all : common utils hostdiscover filetransfer httpserver timeserver doc

#common
srcdir=src/
objdir=obj/
common_objs_list = gollum2411.o
common_objs=$(addprefix $(objdir),$(common_objs_list))
common : $(common_objs)
$(objdir)%.o : $(srcdir)%.cpp | $(objdir)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $^
$(objdir) :
	mkdir -p obj

# utils
utils_dir=utils/
utils_objdir=utils/obj/
utils_objs_list=utils.o
utils_objs=$(addprefix $(utils_objdir),$(utils_objs_list))
utils : $(utils_objs)
$(utils_objdir):
	mkdir -p $@
$(utils_objdir)%.o : $(utils_dir)%.cpp | $(utils_objdir)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $^

#hostdiscover
hostdiscover_dir=hostdiscover/
hostdiscover_bindir=$(hostdiscover_dir)bin/
hostdiscover_objdir=$(hostdiscover_dir)obj/
hostdiscover_objs_list=main.o
hostdiscover_objs=$(addprefix $(hostdiscover_objdir),$(hostdiscover_objs_list))
hostdiscover_bin=$(hostdiscover_bindir)hostdiscover
hostdiscover : $(hostdiscover_bin)
$(hostdiscover_bin) : $(common_objs) $(utils_objs) $(hostdiscover_objs) | $(hostdiscover_bindir)
	$(CC) $(CLAGS) -o $@ $^
$(hostdiscover_objdir)%.o : $(hostdiscover_dir)%.cpp | $(hostdiscover_objdir)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $^
$(hostdiscover_bindir) :
	mkdir -p $@
$(hostdiscover_objdir) :
	mkdir -p $@

#filetransfer
filetransfer_dir=filetransfer/
filetransfer_bindir=$(filetransfer_dir)bin/
filetransfer_objdir=$(filetransfer_dir)obj/
filetransfer_objs_list=client.o filetransfer.o server.o
filetransfer_objs=$(addprefix $(filetransfer_objdir),$(filetransfer_objs_list))
filetransfer_bin=$(filetransfer_bindir)filetransfer
filetransfer:$(filetransfer_bin)
$(filetransfer_bin) : $(filetransfer_objs) $(utils_objs) $(common_objs) | $(filetransfer_bindir)
	$(CC) $(CFLAGS) -o $@ $^
$(filetransfer_objdir)%.o : $(filetransfer_dir)%.cpp | $(filetransfer_objdir)
	$(CC) $(CFLAGS) $(INCLUDES) -I$(filetransfer_dir) -c -o $@ $^
$(filetransfer_objdir) :
	mkdir -p $@
$(filetransfer_bindir) :
	mkdir -p $@

#httpserver
httpserver_dir=httpserver/
httpserver_bindir=$(httpserver_dir)bin/
httpserver_objdir=$(httpserver_dir)obj/
httpserver_objs_list=HTTPServer.o main.o
httpserver_objs=$(addprefix $(httpserver_objdir),$(httpserver_objs_list))
httpserver_bin=$(httpserver_bindir)httpserver
httpserver:$(httpserver_bin)
$(httpserver_bin) : $(httpserver_objs) $(utils_objs) $(common_objs) | $(httpserver_bindir)
	$(CC) $(CFLAGS) -o $@ $^
$(httpserver_objdir)%.o : $(httpserver_dir)%.cpp | $(httpserver_objdir)
	$(CC) $(CFLAGS) $(INCLUDES) -I$(httpserver_dir) -c -o $@ $^
$(httpserver_objdir) :
	mkdir -p $@
$(httpserver_bindir) :
	mkdir -p $@

#timeserver
timeserver_dir=timeserver/
timeserver_bindir=$(timeserver_dir)bin/
timeserver_objdir=$(timeserver_dir)obj/
timeserver_objs_list=timeserver.o
timeserver_objs=$(addprefix $(timeserver_objdir),$(timeserver_objs_list))
timeserver_bin=$(timeserver_bindir)timeserver
timeserver:$(timeserver_bin)
$(timeserver_bin) : $(timeserver_objs) $(utils_objs) $(common_objs) | $(timeserver_bindir)
	$(CC) $(CFLAGS) -o $@ $^
$(timeserver_objdir)%.o : $(timeserver_dir)%.cpp | $(timeserver_objdir)
	$(CC) $(CFLAGS) $(INCLUDES) -I$(timeserver_dir) -c -o $@ $^
$(timeserver_objdir) :
	mkdir -p $@
$(timeserver_bindir) :
	mkdir -p $@

#doc
docdir=doc/
doxyfile = ./sock2014.dox
doc : | $(docdir)
	@echo Generating doxygen documentation
	doxygen $(doxyfile) 2>&1 > /dev/null
$(docdir) :
	mkdir -p $@
cleandoc :
	rm -rf $(docdir)

#clean
clean : cleandoc
	@for d in ${subprojs}; do make -C $$d clean ; done
	rm -rf $(objdir) $(utils_objdir) \
	    $(hostdiscover_objdir) $(hostdiscover_bindir) \
	    $(filetransfer_objdir) $(filetransfer_bindir) \
	    $(httpserver_objdir) $(httpserver_bindir) \
	    $(timeserver_objdir) $(timeserver_bindir)


.PHONY : all $(subprojs) common
