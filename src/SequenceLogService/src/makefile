CC = gcc

CFLAGS = \
	-I../../../include \
	-I../../include \
	-I/usr/lib/jvm/java-6-openjdk/include

#CFLAGS += -D_DEBUG

LDFLAGS = -lslog -lstdc++ -lrt

objs = \
	main.o \
	SequenceLogService.o \
	SequenceLogServiceMain.o \
	SequenceLogServiceWebServer.o

slogsvc: $(objs)
	$(CC) -o slogsvc $^ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) -c $<

install:
	cp slogsvc /usr/bin/

clean:
	$(RM) slogsvc $(objs) depend.inc

depend: $(objs:.o=.cpp)
	-@ $(RM) depend.inc
	-@ for i in $^; do \
		cpp $(CFLAGS) -MM $$i | sed "s/\ [_a-zA-Z0-9][_a-zA-Z0-9]*\.cpp//g" >> depend.inc;\
	done

-include depend.inc
