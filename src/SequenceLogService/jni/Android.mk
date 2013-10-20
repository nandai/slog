LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE	:= slogsvc

LOCAL_SRC_FILES	:= \
	../src/main.cpp \
	../src/getSequenceLogListJson.cpp \
	../src/GetLogResponse.cpp \
	../src/SequenceLogService.cpp \
	../src/SequenceLogServiceMain.cpp \
	../src/SequenceLogServiceWebServer.cpp \
	../src/SequenceLogServiceWebServerResponse.cpp

LOCAL_C_INCLUDES += \
	../../include \
	../../../include

#OCAL_CPPFLAGS   := -fexceptions
LOCAL_CPPFLAGS   := -fexceptions --rtti -D__EXEC__ -std=gnu++0x
#OCAL_CPPFLAGS   += -D_DEBUG

LOCAL_LDLIBS := \
	-L../../../bin/Android \
	-llog \
	-lssl \
	-lcrypto \
	../../../bin/Android/libslog.a

include $(BUILD_EXECUTABLE)
#nclude $(BUILD_SHARED_LIBRARY)
