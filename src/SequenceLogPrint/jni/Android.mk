LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := slogprint

LOCAL_SRC_FILES := \
    ../src/main.cpp

LOCAL_C_INCLUDES += ../../../include
LOCAL_CPPFLAGS   := -D_ANDROID -fexceptions

LOCAL_LDLIBS := \
    -L$(ANDROID_NDK_ROOT)/sources/cxx-stl/stlport/libs/armeabi \
    -lstlport \
    -lslog

include $(BUILD_EXECUTABLE)
