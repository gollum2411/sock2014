CC?=g++
CFLAGS?=-I. -Wall -Werror

objdir=obj/
objs_list=utils.o
objs=$(addprefix $(objdir),$(objs_list))
$(objs) : | $(objdir)

ifeq ($(DEBUG), 1)
CFLAGS += -DDEBUG -g
endif

all : $(objs)

$(objdir)%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(objdir) :
	@mkdir -p $@

clean :
	rm -rf $(objdir)
