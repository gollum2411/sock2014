CC?=gcc
CFLAGS?=-I.

bin_name=timeserver

ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -g
endif

all : $(bin_name)

$(bin_name) : timeserver.c
	$(CC) $(CFLAGS) -o $@ $^
	@echo Done

clean :
	rm -rf $(bin_name)

.PHONY : clean
