LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := slog

LOCAL_SRC_FILES := \
    ../src/slog.cpp \
    ../src/Buffer.cpp \
    ../src/ByteBuffer.cpp \
    ../src/CoreString.cpp \
    ../src/DateTime.cpp \
    ../src/Dir.cpp \
    ../src/Exception.cpp \
    ../src/File.cpp \
    ../src/FileFind.cpp \
    ../src/FileInfo.cpp \
    ../src/HttpRequest.cpp \
    ../src/HttpResponse.cpp \
    ../src/Json.cpp \
    ../src/Mutex.cpp \
    ../src/Process.cpp \
    ../src/Socket.cpp \
    ../src/Thread.cpp \
    ../src/Tokenizer.cpp \
    ../src/Util.cpp \
    ../src/WebServerThread.cpp \
    ../src/WebServerResponseThread.cpp \
    ../src/WebSocket.cpp \
    ../src/WebSocketClient.cpp \
    ../src/SequenceLog.cpp \
    ../src/jp_printf_slog_Log.cpp \
    ../src/sha1.c

LOCAL_C_INCLUDES += \
    $(ANDROID_NDK_ROOT)/sources/cxx-stl/stlport/stlport \
    . \
    ../../include \
    ../../../include

LOCAL_CPPFLAGS   := -D__SLOG__ -D__STATIC_LIBRARY__ -fexceptions --rtti
#OCAL_CPPFLAGS   += -D_DEBUG

LOCAL_LDLIBS := \
    -L$(ANDROID_NDK_ROOT)/sources/cxx-stl/stlport/libs/armeabi \
    -L../../../bin/Android \
    -llog \
    -lstlport \
    -lssl \
    -lcrypto

include $(BUILD_STATIC_LIBRARY)
