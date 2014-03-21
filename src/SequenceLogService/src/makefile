CC = gcc

CFLAGS = \
	-I../../../include \
	-I../../include \
	-I/usr/lib/jvm/java-6-openjdk/include \
	-std=c++0x

#CFLAGS += -D_DEBUG
#CFLAGS += -D__SLOG__

LDFLAGS = -lslog -lstdc++ -lrt -lssl -lpthread -ldl

INSTALLDIR = /usr/bin

objs = \
	main.o \
	getSequenceLogListJson.o \
	Account.o \
	AccountResponse.o \
	GetLogResponse.o \
	SequenceLogService.o \
	SequenceLogServiceMain.o \
	SequenceLogServiceWebServer.o \
	SequenceLogServiceWebServerResponse.o \
	SQLite.o \
	sqlite3/sqlite3.o

slogsvc: $(objs)
	$(CC) -o slogsvc $^ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) -c $<

install:
	cp slogsvc                       $(INSTALLDIR)/
	mkdir -p                         $(INSTALLDIR)/SequenceLogServiceWeb
	cp ../SequenceLogServiceWeb/*.*  $(INSTALLDIR)/SequenceLogServiceWeb/
	cp ../SequenceLogService.db      $(INSTALLDIR)/
	chmod 666 $(INSTALLDIR)/SequenceLogService.db

clean:
	$(RM) slogsvc $(objs) depend.inc

depend: $(objs:.o=.cpp)
	-@ $(RM) depend.inc
	-@ for i in $^; do \
		cpp $(CFLAGS) -MM $$i | sed "s/\ [_a-zA-Z0-9][_a-zA-Z0-9]*\.cpp//g" >> depend.inc;\
	done

-include depend.inc
