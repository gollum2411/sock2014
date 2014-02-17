subprojs = utils timeserver filetransfer

all : $(subprojs)

$(subprojs) :
	make -C $@

clean :
	@for d in ${subprojs}; do make -C $$d clean ; done

.PHONY : all $(subprojs)