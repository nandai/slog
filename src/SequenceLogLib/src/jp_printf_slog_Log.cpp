﻿/*
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
#include "slog/WebSocketClient.h"
#include "slog/ByteBuffer.h"

#if defined(__ANDROID__)
    #include <android/log.h>
#endif

using namespace slog;

static JavaVM* JVM = NULL;
static jint JNI_VERSION = JNI_VERSION_1_6;

struct _jmethodID {};   // warning LINK4248 対策
struct _jfieldID  {};   // 〃

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

/*!
 *  \brief  JNIEnv取得
 */
static JNIEnv* getJNIEnv()
{
    JNIEnv* env = NULL;
    jint version = JNI_VERSION;

    if (JVM->GetEnv((void**)&env, version) == JNI_OK)
        return env;

    if (JVM->AttachCurrentThread((void**)&env, NULL) == JNI_OK)
		return env;

    return NULL;
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
 * Signature: (Ljava/lang/String;)V
 */
static void JNICALL setServiceAddress(JNIEnv* env, jclass, jstring aAddress)
{
    JavaString address(env, aAddress);
    setSequenceLogServiceAddress(address.getBuffer());
}

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

/*!
 * Java用WebSocketClientクラス
 */
class JavaWebSocketClient : public WebSocketClient, public WebSocketListener
{
public:     static jfieldID     mNativeObj;
private:    static jmethodID    mOnOpen;
            static jmethodID    mOnError;
            static jmethodID    mOnMessage;
            static jmethodID    mOnClose;

            jobject             mJavaObj;

            /*!
             * コンストラクタ／デストラクタ
             */
public:     JavaWebSocketClient(JNIEnv* env, jobject javaObj);
            virtual ~JavaWebSocketClient();

            /*!
             * ハンドラ
             */
public:     virtual void onOpen();
            virtual void onError(const char* message);
            virtual void onMessage(const ByteBuffer& buffer);
            virtual void onClose();
};

/*!
 * コンストラクタ
 */
JavaWebSocketClient::JavaWebSocketClient(JNIEnv* env, jobject javaObj) : WebSocketClient()
{
    jclass clazz = env->GetObjectClass(javaObj);

    mNativeObj = env->GetFieldID( clazz, "mNativeObj", "J");
    mOnOpen =    env->GetMethodID(clazz, "onOpen",     "()V");
    mOnError =   env->GetMethodID(clazz, "onError",    "(Ljava/lang/String;)V");
    mOnMessage = env->GetMethodID(clazz, "onMessage",  "(Ljava/nio/ByteBuffer;)V");
    mOnClose =   env->GetMethodID(clazz, "onClose",    "()V");

    mJavaObj = env->NewGlobalRef(javaObj);
    setListener(this);

    env->SetLongField(mJavaObj, mNativeObj, (jlong)this);
}

/*!
 * デストラクタ
 */
JavaWebSocketClient::~JavaWebSocketClient()
{
    JNIEnv* env = getJNIEnv();
    env->SetLongField(mJavaObj, mNativeObj, NULL);
}

/*!
 * onOpen
 */
void JavaWebSocketClient::onOpen()
{
    JNIEnv* env = getJNIEnv();
    env->CallVoidMethod(mJavaObj, mOnOpen);
}

/*!
 * onError
 */
void JavaWebSocketClient::onError(const char* message)
{
    JNIEnv* env = getJNIEnv();

    if (env == NULL)
        return;

#if defined(_WINDOWS)
    UTF16LE utf16le;
    utf16le.conv(message, 1);
#else
    UTF16LE utf16le;
    utf16le.conv(message, 0);
#endif

//  jstring str = env->NewStringUTF(message);
    jstring str = env->NewString((jchar*)utf16le.getBuffer(), utf16le.getChars());

    env->CallVoidMethod(mJavaObj, mOnError, str);
    env->ReleaseStringUTFChars(str, message);
}

/*!
 * onMessage
 */
void JavaWebSocketClient::onMessage(const ByteBuffer& buffer)
{
    JNIEnv* env = getJNIEnv();

    if (env == NULL)
        return;

    jobject buf = env->NewDirectByteBuffer(buffer.getBuffer(), buffer.getLength());
    buf = env->NewGlobalRef(buf);

    env->CallVoidMethod(mJavaObj, mOnMessage, buf);
    env->DeleteGlobalRef(buf);
}

/*!
 * onClose
 */
void JavaWebSocketClient::onClose()
{
    JNIEnv* env = getJNIEnv();
    env->CallVoidMethod(mJavaObj, mOnClose);
}

/*!
 * スタティック変数初期化
 */
jfieldID  JavaWebSocketClient::mNativeObj = NULL;
jmethodID JavaWebSocketClient::mOnOpen =    NULL;
jmethodID JavaWebSocketClient::mOnError =   NULL;
jmethodID JavaWebSocketClient::mOnMessage = NULL;
jmethodID JavaWebSocketClient::mOnClose =   NULL;

/*!
 * JavaWebSocketClient取得
 */
static JavaWebSocketClient* getClient(JNIEnv* env, jobject thiz)
{
    JavaWebSocketClient* client = (JavaWebSocketClient*)env->GetLongField(thiz, JavaWebSocketClient::mNativeObj);
    return client;
}

/*!
 *  \brief  接続
 */
static void JNICALL ws_open(JNIEnv* env, jobject thiz, jstring aURL)
{
    JavaWebSocketClient* client = new JavaWebSocketClient(env, thiz);
    JavaString url(env, aURL);

    try
    {
        client->connect(url);
    }
    catch (Exception e)
    {
        client->onError(e.getMessage());
    }
}

/*!
 *  \brief  切断
 */
static void JNICALL ws_close(JNIEnv* env, jobject thiz)
{
    JavaWebSocketClient* client = getClient(env, thiz);
    client->close();
}

/*!
 *  \brief  テキスト送信
 */
static void JNICALL ws_send(JNIEnv* env, jobject thiz, jstring astr)
{
    JavaWebSocketClient* client = getClient(env, thiz);
    JavaString str(env, astr);
    client->send(str);
}

// JNIメソッド配列
static JNINativeMethodEx sSlogMethods[] =
{
    {"setFileName",       "(Ljava/lang/String;)V",                    (void*)setFileName      },
    {"setServiceAddress", "(Ljava/lang/String;)V",                    (void*)setServiceAddress},
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

// JNIメソッド配列
static JNINativeMethod sWebSocketMethods[] =
{
    {"open",  "(Ljava/lang/String;)V", (void*)ws_open },
    {"close", "()V",                   (void*)ws_close},
    {"send",  "(Ljava/lang/String;)V", (void*)ws_send },
};

/*!
 *  \brief  Java Native Interface OnLoad
 */
extern "C" jint slog_JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JVM = vm;
    JNIEnv* env = getJNIEnv();

    if (env == NULL)
    {
        noticeLog("cannot get JNIEnv.");
        return -1;
    }

    if (registerNatives(env, "jp/printf/slog/Log", (JNINativeMethod*)sSlogMethods, sizeof(sSlogMethods) / sizeof(sSlogMethods[0])) == false)
    {
        noticeLog("jp/printf/slog/Log regist failed.");
        return -1;
    }

    if (registerNatives(env, "jp/printf/WebSocketClient", (JNINativeMethod*)sWebSocketMethods, sizeof(sWebSocketMethods) / sizeof(sWebSocketMethods[0])) == false)
    {
        noticeLog("jp/printf/WebSocketClient regist failed.");
        return -1;
    }

    return JNI_VERSION;
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
