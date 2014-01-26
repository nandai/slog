CC = gcc

CFLAGS = \
	-D__SLOG__ \
	-I../../../include \
	-I../../include \
	-I$(JAVA_HOME)/include \
	-I$(JAVA_HOME)/include/linux \
	-fPIC \
	-std=c++0x

#CFLAGS += -D_DEBUG

LDFLAGS = -lrt -lssl -lstdc++ -shared

INSTALLDIR = /usr/lib

objs = \
	slog.o \
	Buffer.o \
	ByteBuffer.o \
	CoreString.o \
	DateTime.o \
	Dir.o \
	Exception.o \
	File.o \
	FileFind.o \
	FileInfo.o \
	HtmlGenerator.o \
	HttpRequest.o \
	HttpResponse.o \
	Json.o \
	MimeType.o \
	Mutex.o \
	PointerString.o \
	Process.o \
	Socket.o \
	String.o \
	Thread.o \
	Tokenizer.o \
	Util.o \
	Variable.o \
	WebServerThread.o \
	WebServerResponseThread.o \
	WebSocket.o \
	WebSocketClient.o \
	SequenceLog.o \
	jp_printf_slog_Log.o \
	sha1.o

#all: depend libslog.so

libslog.so: $(objs)
	$(CC) -o libslog.so $^ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) -c $<

install:
#	mkdir -p      ../../../bin/linux/x64/
#	cp libslog.so ../../../bin/linux/x64/
	cp libslog.so $(INSTALLDIR)/libslog.so.1.2.6
	chmod 644     $(INSTALLDIR)/libslog.so.1.2.6
	ln -sf        $(INSTALLDIR)/libslog.so.1.2.6 $(INSTALLDIR)/libslog.so.1
	ln -sf        $(INSTALLDIR)/libslog.so.1     $(INSTALLDIR)/libslog.so

	cp -rf ../../../include/slog /usr/include/

clean:
	$(RM) libslog.so $(objs) depend.inc

depend: $(objs:.o=.cpp)
	-@ $(RM) depend.inc
	-@ for i in $^; do \
		cpp $(CFLAGS) -MM $$i | sed "s/\ [_a-zA-Z0-9][_a-zA-Z0-9]*\.cpp//g" >> depend.inc;\
	done

-include depend.inc
