LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := slog

LOCAL_SRC_FILES := \
    ../src/slog.cpp \
    ../src/Buffer.cpp \
    ../src/ByteBuffer.cpp \
    ../src/Convert.cpp \
    ../src/CoreString.cpp \
    ../src/DateTime.cpp \
    ../src/Dir.cpp \
    ../src/Exception.cpp \
    ../src/File.cpp \
    ../src/FileFind.cpp \
    ../src/FileInfo.cpp \
    ../src/HtmlGenerator.cpp \
    ../src/HttpRequest.cpp \
    ../src/HttpResponse.cpp \
    ../src/Json.cpp \
    ../src/MimeType.cpp \
    ../src/Mutex.cpp \
    ../src/PointerString.cpp \
    ../src/Process.cpp \
    ../src/Socket.cpp \
    ../src/String.cpp \
    ../src/Thread.cpp \
    ../src/Tokenizer.cpp \
    ../src/Util.cpp \
    ../src/Variable.cpp \
    ../src/WebServerThread.cpp \
    ../src/WebServerResponseThread.cpp \
    ../src/WebSocket.cpp \
    ../src/WebSocketClient.cpp \
    ../src/SequenceLog.cpp \
    ../src/jp_printf_slog_Log.cpp \
    ../src/sha1.c

LOCAL_C_INCLUDES += \
    . \
    ../../include \
    ../../../include

LOCAL_CPPFLAGS   := -D__SLOG__ -D__SHARED_LIBRARY__ -fexceptions --rtti -std=gnu++0x
#OCAL_CPPFLAGS   += -D_DEBUG

LOCAL_LDLIBS := \
    -L../../../bin/Android \
    -llog \
    -lssl \
    -lcrypto

include $(BUILD_SHARED_LIBRARY)
