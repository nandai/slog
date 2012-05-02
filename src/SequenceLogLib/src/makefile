CC = gcc

CFLAGS = \
	-D__SLOG__ \
	-I../../../include \
	-I../../include \
	-I$(JAVA_HOME)/include \
	-I$(JAVA_HOME)/include/linux \
	-fPIC

#CFLAGS += -D_DEBUG

LDFLAGS = -lrt -shared

INSTALLDIR = /usr/lib

objs = \
	slog.o \
	Buffer.o \
	ByteBuffer.o \
	CoreString.o \
	Exception.o \
	File.o \
	FileFind.o \
	FileInfo.o \
	Socket.o \
	Thread.o \
	Tokenizer.o \
	SequenceLog.o \
	net_log_tools_slog_Log.o

#all: depend libslog.so

libslog.so: $(objs)
	$(CC) -o libslog.so $^ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) -c $<

install:
#	mkdir -p      ../../../bin/linux/x64/
#	cp libslog.so ../../../bin/linux/x64/
	cp libslog.so $(INSTALLDIR)/libslog.so.1.0.0
	chmod 644     $(INSTALLDIR)/libslog.so.1.0.0
	ln -sf        $(INSTALLDIR)/libslog.so.1.0.0 $(INSTALLDIR)/libslog.so.1
	ln -sf        $(INSTALLDIR)/libslog.so.1     $(INSTALLDIR)/libslog.so

clean:
	$(RM) libslog.so $(objs) depend.inc

depend: $(objs:.o=.cpp)
	-@ $(RM) depend.inc
	-@ for i in $^; do \
		cpp $(CFLAGS) -MM $$i | sed "s/\ [_a-zA-Z0-9][_a-zA-Z0-9]*\.cpp//g" >> depend.inc;\
	done

-include depend.inc
