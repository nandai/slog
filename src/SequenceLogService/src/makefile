CC = gcc

CFLAGS = \
	-I../../../include \
	-I../../include \
	-I/usr/lib/jvm/java-6-openjdk/include

#CFLAGS += -D_DEBUG
#CFLAGS += -D__SLOG__

LDFLAGS = -lslog -lstdc++ -lrt -lssl

INSTALLDIR = /usr/bin

objs = \
	main.o \
	getSequenceLogListJson.o \
	GetLogResponse.o \
	SequenceLogService.o \
	SequenceLogServiceMain.o \
	SequenceLogServiceWebServer.o \
	SequenceLogServiceWebServerResponse.o

slogsvc: $(objs)
	$(CC) -o slogsvc $^ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) -c $<

install:
	cp slogsvc                       $(INSTALLDIR)/
	mkdir -p                         $(INSTALLDIR)/SequenceLogServiceWeb
	cp  ../SequenceLogServiceWeb/*.* $(INSTALLDIR)/SequenceLogServiceWeb/

clean:
	$(RM) slogsvc $(objs) depend.inc

depend: $(objs:.o=.cpp)
	-@ $(RM) depend.inc
	-@ for i in $^; do \
		cpp $(CFLAGS) -MM $$i | sed "s/\ [_a-zA-Z0-9][_a-zA-Z0-9]*\.cpp//g" >> depend.inc;\
	done

-include depend.inc
