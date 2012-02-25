CC = gcc

CFLAGS = \
	-I../../../include \
	-I../../include \
        -I/usr/lib/jvm/java-6-openjdk/include

LDFLAGS = -lslog -lstdc++ -lrt

objs = \
	main.o

slogprint: $(objs)
	$(CC) -o slogprint $^ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) -c $<

install:
	cp slogprint /usr/bin/

clean:
	$(RM) slogprint $(objs) depend.inc

depend: $(objs:.o=.cpp)
	-@ $(RM) depend.inc
	-@ for i in $^; do \
		cpp $(CFLAGS) -MM $$i | sed "s/\ [_a-zA-Z0-9][_a-zA-Z0-9]*\.cpp//g" >> depend.inc;\
	done

-include depend.inc

