LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := slog

LOCAL_SRC_FILES := \
    ../src/slog.cpp \
    ../src/Buffer.cpp \
    ../src/ByteBuffer.cpp \
    ../src/CoreString.cpp \
    ../src/Exception.cpp \
    ../src/File.cpp \
    ../src/FileFind.cpp \
    ../src/FileInfo.cpp \
    ../src/HttpRequest.cpp \
    ../src/HttpResponse.cpp \
    ../src/Json.cpp \
    ../src/Socket.cpp \
    ../src/Thread.cpp \
    ../src/Tokenizer.cpp \
    ../src/Util.cpp \
    ../src/WebServerThread.cpp \
    ../src/WebServerResponseThread.cpp \
    ../src/SequenceLog.cpp \
    ../src/jp_printf_slog_Log.cpp \
    ../src/sha1.c

LOCAL_C_INCLUDES += \
    $(ANDROID_NDK_ROOT)/sources/cxx-stl/stlport/stlport \
    ../../include \
    ../../../include

LOCAL_CPPFLAGS   := -D__SLOG__ -D__STATIC_LIBRARY__ -fexceptions --rtti
#OCAL_CPPFLAGS   += -D_DEBUG

LOCAL_LDLIBS := \
    -L$(ANDROID_NDK_ROOT)/sources/cxx-stl/stlport/libs/armeabi \
    -llog \
    -lstlport

include $(BUILD_STATIC_LIBRARY)
