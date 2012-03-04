/*
 * Copyright (C) 2011 log-tools.net
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
 *  \file   net_log_tools_slog_Log.cpp
 *  \brief  シーケンスログ (JNI)
 *  \author Copyright 2011 log-tools.net
 */
#include "slog/SequenceLog.h"
#include "slog/JavaString.h"

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
        return false;

    if (env->RegisterNatives(clazz, methods, numMethods) == 0)
        result = true;

    env->DeleteLocalRef(clazz);
    return result;
}

/*
 * Class:     net_log_tools_slog_Log
 * Method:    setFileName
 * Signature: (Ljava/lang/String;)V
 */
static void JNICALL setFileName(JNIEnv* env, jclass, jstring aFileName)
{
    JavaString fileName(env, aFileName);
    setSequenceLogFileName(fileName.getBuffer());
}

/*
 * Class:     net_log_tools_slog_Log
 * Method:    setRootFlag
 * Signature: (I)V
 */
static void JNICALL setRootFlagJNI(JNIEnv* env, jclass, jint outputFlag)
{
    setRootFlag(outputFlag);
}

/*
 * Class:     net_log_tools_slog_Log
 * Method:    stepIn
 * Signature: (Ljava/lang/String;Ljava/lang/String;I)J
 */
static jlong JNICALL stepIn1(JNIEnv* env, jclass, jstring aClassName, jstring aFuncName, jint outputFlag)
{
    JavaString className(env, aClassName);
    JavaString funcName( env, aFuncName);

    SequenceLog* slogObj = new SequenceLog(className.getBuffer(), funcName.getBuffer(), (SequenceLogOutputFlag)outputFlag);
    return (jlong)slogObj;
}

/*
 * Class:     net_log_tools_slog_Log
 * Method:    stepIn
 * Signature: (ILjava/lang/String;I)J
 */
static jlong JNICALL stepIn2(JNIEnv* env, jclass, jint classID, jstring aFuncName, jint outputFlag)
{
    JavaString funcName( env, aFuncName);

    SequenceLog* slogObj = new SequenceLog(classID, funcName.getBuffer(), (SequenceLogOutputFlag)outputFlag);
    return (jlong)slogObj;
}

/*
 * Class:     net_log_tools_slog_Log
 * Method:    stepIn
 * Signature: (III)J
 */
static jlong JNICALL stepIn3(JNIEnv* env, jclass, jint classID, jint funcID, jint outputFlag)
{
    SequenceLog* slogObj = new SequenceLog(classID, funcID, (SequenceLogOutputFlag)outputFlag);
    return (jlong)slogObj;
}

/*
 * Class:     net_log_tools_slog_Log
 * Method:    stepOut
 * Signature: (J)V
 */
static void JNICALL stepOut(JNIEnv* env, jclass, jlong slog)
{
    SequenceLog* slogObj = (SequenceLog*)slog;
    delete slogObj;
}

/*
 * Class:     net_log_tools_slog_Log
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
 * Class:     net_log_tools_slog_Log
 * Method:    message
 * Signature: (IIJ)V
 */
static void JNICALL message2(JNIEnv* env, jclass, jint level, jint messageID, jlong slog)
{
    SequenceLog* slogObj = (SequenceLog*)slog;
    slogObj->message((SequenceLogLevel)level, messageID);
}

// JNIメソッド配列
static JNINativeMethodEx sMethods[] =
{
    {"setFileName", "(Ljava/lang/String;)V",                    (void*)setFileName   },
    {"setRootFlag", "(I)V",                                     (void*)setRootFlagJNI},
    {"stepIn",      "(Ljava/lang/String;Ljava/lang/String;I)J", (void*)stepIn1       },
    {"stepIn",      "(ILjava/lang/String;I)J",                  (void*)stepIn2       },
    {"stepIn",      "(III)J",                                   (void*)stepIn3       },
    {"stepOut",     "(J)V",                                     (void*)stepOut       },
    {"message",     "(ILjava/lang/String;J)V",                  (void*)message1      },
    {"message",     "(IIJ)V",                                   (void*)message2      },
};

/*!
 *  \brief  Java Native Interface OnLoad
 */
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint version = JNI_VERSION_1_6;

    if (vm->GetEnv((void**)&env, version) != JNI_OK)
        return -1;

    if (registerNatives(env, "net/log_tools/slog/Log", (JNINativeMethod*)sMethods, sizeof(sMethods) / sizeof(sMethods[0])) == false)
        return -1;

    return version;
}

#if defined(_WINDLL) || defined(__SHARED_LIBRARY__)
/*
 * シーケンスログファイル名取得
 */
const char* getSequenceLogFileName()
{
    TRACE("getSequenceLogFileName() in net_log_tools_slog_Log.cpp\n", 0);
    return NULL;
}

#if defined(_WINDOWS)
#include "slog/Socket.h"

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
