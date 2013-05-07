/*
 * Copyright (C) 2011-2013 printf.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*!
 *  \file   jp_printf_slog_Log.cpp
 *  \brief  シーケンスログ (JNI)
 *  \author Copyright 2011-2013 printf.jp
 */
#include "slog/SequenceLog.h"
#include "slog/JavaString.h"

#if defined(__ANDROID__)
    #include <android/log.h>
#endif

using namespace slog;

// gccの警告「warning: deprecated conversion from string constant to 'char*'」に対処するため、
// const char*版JNINativeMethodを定義する
struct JNINativeMethodEx
{
    const char* name;
    const char* signature;
    void*       fnPtr;
};

/*!
 *  \brief  JNIメソッド登録
 */
static bool registerNatives(JNIEnv* env, const char* className, const JNINativeMethod* methods, int numMethods)
{
    jclass clazz = env->FindClass(className);
    bool result = false;

    if (clazz == NULL)
    {
        noticeLog("env->FindClass() failed.");
        return false;
    }

    if (env->RegisterNatives(clazz, methods, numMethods) == 0)
        result = true;

    env->DeleteLocalRef(clazz);
    return result;
}

/*
 * Class:     jp_printf_slog_Log
 * Method:    setFileName
 * Signature: (Ljava/lang/String;)V
 */
static void JNICALL setFileName(JNIEnv* env, jclass, jstring aFileName)
{
    JavaString fileName(env, aFileName);
    setSequenceLogFileName(fileName.getBuffer());
}

/*
 * Class:     jp_printf_slog_Log
 * Method:    setServiceAddress
 * Signature: (Ljava/lang/String;I)V
 */
static void JNICALL setServiceAddress(JNIEnv* env, jclass, jstring aAddress, jint port)
{
    JavaString address(env, aAddress);
    setSequenceLogServiceAddress(address.getBuffer(), (uint16_t)port);
}

/*
 * Class:     jp_printf_slog_Log
 * Method:    setRootFlag
 * Signature: (I)V
 */
//static void JNICALL setRootFlagJNI(JNIEnv* env, jclass, jint outputFlag)
//{
//    setRootFlag(outputFlag);
//}

/*
 * Class:     jp_printf_slog_Log
 * Method:    enableOutput
 * Signature: (Z)V
 */
static void JNICALL enableOutputJNI(JNIEnv* env, jclass, jboolean enable)
{
    enableOutput(enable == 1);
}

/*
 * Class:     jp_printf_slog_Log
 * Method:    stepIn
 * Signature: (Ljava/lang/String;Ljava/lang/String;I)J
 */
static jlong JNICALL stepIn1(JNIEnv* env, jclass, jstring aClassName, jstring aFuncName)//, jint outputFlag)
{
    JavaString className(env, aClassName);
    JavaString funcName( env, aFuncName);

    SequenceLog* slogObj = new SequenceLog(className.getBuffer(), funcName.getBuffer());//, (SequenceLogOutputFlag)outputFlag);
    return (jlong)slogObj;
}

/*
 * Class:     jp_printf_slog_Log
 * Method:    stepIn
 * Signature: (ILjava/lang/String;I)J
 */
//static jlong JNICALL stepIn2(JNIEnv* env, jclass, jint classID, jstring aFuncName)//, jint outputFlag)
//{
//    JavaString funcName( env, aFuncName);
//
//    SequenceLog* slogObj = new SequenceLog(classID, funcName.getBuffer());//, (SequenceLogOutputFlag)outputFlag);
//    return (jlong)slogObj;
//}

/*
 * Class:     jp_printf_slog_Log
 * Method:    stepIn
 * Signature: (III)J
 */
//static jlong JNICALL stepIn3(JNIEnv* env, jclass, jint classID, jint funcID)//, jint outputFlag)
//{
//    SequenceLog* slogObj = new SequenceLog(classID, funcID);//, (SequenceLogOutputFlag)outputFlag);
//    return (jlong)slogObj;
//}

/*
 * Class:     jp_printf_slog_Log
 * Method:    stepOut
 * Signature: (J)V
 */
static void JNICALL stepOut(JNIEnv* env, jclass, jlong slog)
{
    SequenceLog* slogObj = (SequenceLog*)slog;
    delete slogObj;
}

/*
 * Class:     jp_printf_slog_Log
 * Method:    message
 * Signature: (ILjava/lang/String;J)V
 */
static void JNICALL message1(JNIEnv* env, jclass, jint level, jstring aMessage, jlong slog)
{
    SequenceLog* slogObj = (SequenceLog*)slog;
    JavaString message(env, aMessage);

    slogObj->message((SequenceLogLevel)level, "%s", message.getBuffer());
}

/*
 * Class:     jp_printf_slog_Log
 * Method:    message
 * Signature: (IIJ)V
 */
//static void JNICALL message2(JNIEnv* env, jclass, jint level, jint messageID, jlong slog)
//{
//    SequenceLog* slogObj = (SequenceLog*)slog;
//    slogObj->message((SequenceLogLevel)level, messageID);
//}

/*
 * Class:     jp_printf_slog_Log
 * Method:    message
 * Signature: (ILjava/lang/String;Ljava/lang/String)V
 */
static void JNICALL message3(JNIEnv* env, jclass, jint level, jstring aMessage, jstring aTag)
{
#if defined(__ANDROID__)
    JavaString message(env, aMessage);
    JavaString tag(    env, aTag);

    int prio = ANDROID_LOG_DEBUG;

    switch (level)
    {
    case DEBUG:
        prio = ANDROID_LOG_DEBUG;
        break;

    case INFO:
        prio = ANDROID_LOG_INFO;
        break;

    case WARN:
        prio = ANDROID_LOG_WARN;
        break;

    case ERROR:
        prio = ANDROID_LOG_ERROR;
        break;
    }

    __android_log_write(prio, tag.getBuffer(),  message.getBuffer());
#endif
}

// JNIメソッド配列
static JNINativeMethodEx sMethods[] =
{
    {"setFileName",       "(Ljava/lang/String;)V",                    (void*)setFileName      },
    {"setServiceAddress", "(Ljava/lang/String;I)V",                   (void*)setServiceAddress},
//  {"setRootFlag",       "(I)V",                                     (void*)setRootFlagJNI   },
    {"enableOutput",      "(Z)V",                                     (void*)enableOutputJNI  },
    {"stepIn",            "(Ljava/lang/String;Ljava/lang/String;)J",  (void*)stepIn1          },
//  {"stepIn",            "(ILjava/lang/String;)J",                   (void*)stepIn2          },
//  {"stepIn",            "(II)J",                                    (void*)stepIn3          },
    {"stepOut",           "(J)V",                                     (void*)stepOut          },
    {"message",           "(ILjava/lang/String;J)V",                  (void*)message1         },
//  {"message",           "(IIJ)V",                                   (void*)message2         },

    {"message",           "(ILjava/lang/String;Ljava/lang/String;)V", (void*)message3         },
};

/*!
 *  \brief  Java Native Interface OnLoad
 */
extern "C" jint slog_JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint version = JNI_VERSION_1_6;

    if (vm->GetEnv((void**)&env, version) != JNI_OK)
    {
        noticeLog("vm->GetEnv() failed.");
        return -1;
    }

    if (registerNatives(env, "jp/printf/slog/Log", (JNINativeMethod*)sMethods, sizeof(sMethods) / sizeof(sMethods[0])) == false)
    {
        noticeLog("registerNatives() failed.");
        return -1;
    }

    return version;
}

//#if defined(_WINDLL) || defined(__SHARED_LIBRARY__)
#if !defined(__STATIC_LIBRARY__)
/*!
 *  \brief  Java Native Interface OnLoad
 */
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
    return slog_JNI_OnLoad(vm, reserved);
}

#if defined(_WINDOWS) && 0
#include "slog/Socket.h"

/*!
 *  \brief  DllMain
 */
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            Socket::startup();
            break;

        case DLL_PROCESS_DETACH:
            Socket::cleanup();
            break;
    }

    return  TRUE;
}
#endif // defined(_WINDOWS)
#endif // defined(_WINDLL) || defined(__SHARED_LIBRARY__)
