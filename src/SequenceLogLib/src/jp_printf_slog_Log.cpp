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
#include "slog/WebSocketClient.h"
#include "slog/ByteBuffer.h"

#if defined(__ANDROID__)
    #include <android/log.h>
#endif

using namespace slog;

static JavaVM* JVM = nullptr;
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

    if (clazz == nullptr)
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
    JNIEnv* env = nullptr;
    jint version = JNI_VERSION;

    if (JVM->GetEnv((void**)&env, version) == JNI_OK)
        return env;

#if defined(__ANDROID__)
    if (JVM->AttachCurrentThread(        &env, nullptr) == JNI_OK)
        return env;
#else
    if (JVM->AttachCurrentThread((void**)&env, nullptr) == JNI_OK)
        return env;
#endif

    return nullptr;
}

/*
 * Class:     jp_printf_slog_Log
 * Method:    loadConfig
 * Signature: (Ljava/lang/String;)V
 */
static void JNICALL loadConfig(JNIEnv* env, jclass, jstring aFileName)
{
    JavaString fileName(env, aFileName);
    loadSequenceLogConfig(fileName.getBuffer());
}

/*
 * Class:     jp_printf_slog_Log
 * Method:    stepIn
 * Signature: (Ljava/lang/String;Ljava/lang/String;I)J
 */
static jlong JNICALL stepIn1(JNIEnv* env, jclass, jstring aClassName, jstring aFuncName)
{
    JavaString className(env, aClassName);
    JavaString funcName( env, aFuncName);

    SequenceLog* slogObj = new SequenceLog(className.getBuffer(), funcName.getBuffer());
    return (jlong)slogObj;
}

/*
 * Class:     jp_printf_slog_Log
 * Method:    stepIn
 * Signature: (ILjava/lang/String;I)J
 */
//static jlong JNICALL stepIn2(JNIEnv* env, jclass, jint classID, jstring aFuncName)
//{
//    JavaString funcName( env, aFuncName);
//
//    SequenceLog* slogObj = new SequenceLog(classID, funcName.getBuffer());
//    return (jlong)slogObj;
//}

/*
 * Class:     jp_printf_slog_Log
 * Method:    stepIn
 * Signature: (III)J
 */
//static jlong JNICALL stepIn3(JNIEnv* env, jclass, jint classID, jint funcID)
//{
//    SequenceLog* slogObj = new SequenceLog(classID, funcID);
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
             * コンストラクタ
             */
public:     JavaWebSocketClient(JNIEnv* env, jobject javaObj);

            /*!
             * デストラクタ
             */
            virtual ~JavaWebSocketClient() override;

            /*!
             * ハンドラ
             */
public:     virtual void onOpen() override;
            virtual void onError(const char* message) override;
            virtual void onMessage(const ByteBuffer& buffer) override;
            virtual void onClose() override;
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
    addWebSocketListener(this);

    env->SetLongField(mJavaObj, mNativeObj, (jlong)this);
}

/*!
 * デストラクタ
 */
JavaWebSocketClient::~JavaWebSocketClient()
{
    JNIEnv* env = getJNIEnv();
    env->SetLongField(mJavaObj, mNativeObj, 0);
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

    if (env == nullptr)
        return;

#if defined(_WINDOWS)
    UTF16LE utf16le;
    utf16le.conv(message);
    jstring str = env->NewString((jchar*)utf16le.getBuffer(), utf16le.getChars());
#else
    jstring str = env->NewStringUTF(message);
#endif

    env->CallVoidMethod(mJavaObj, mOnError, str);
//  env->ReleaseStringUTFChars(str, message);
}

/*!
 * onMessage
 */
void JavaWebSocketClient::onMessage(const ByteBuffer& buffer)
{
    JNIEnv* env = getJNIEnv();

    if (env == nullptr)
        return;

    jobject localBuf = env->NewDirectByteBuffer(buffer.getBuffer(), buffer.getLength());
    jobject buf =      env->NewGlobalRef(localBuf);

    env->CallVoidMethod(mJavaObj, mOnMessage, buf);
    env->DeleteGlobalRef(buf);
    env->DeleteLocalRef(localBuf);
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
jfieldID  JavaWebSocketClient::mNativeObj = nullptr;
jmethodID JavaWebSocketClient::mOnOpen =    nullptr;
jmethodID JavaWebSocketClient::mOnError =   nullptr;
jmethodID JavaWebSocketClient::mOnMessage = nullptr;
jmethodID JavaWebSocketClient::mOnClose =   nullptr;

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

    client->open(&url);
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
static void JNICALL ws_sendText(JNIEnv* env, jobject thiz, jstring aStr)
{
    JavaWebSocketClient* client = getClient(env, thiz);
    JavaString str(env, aStr);
    client->send(&str);
}

/*!
 *  \brief  バイナリ送信
 */
static void JNICALL ws_sendBinary(JNIEnv* env, jobject thiz, jobject data)
{
    JavaWebSocketClient* client = getClient(env, thiz);
    const char* buffer = (const char*)env->GetDirectBufferAddress( data);
    int32_t len =            (int32_t)env->GetDirectBufferCapacity(data);

    client->send(buffer, len);
}

// JNIメソッド配列
static JNINativeMethodEx sSlogMethods[] =
{
    {"loadConfig",        "(Ljava/lang/String;)V",                    (void*)loadConfig       },
    {"stepIn",            "(Ljava/lang/String;Ljava/lang/String;)J",  (void*)stepIn1          },
//  {"stepIn",            "(ILjava/lang/String;)J",                   (void*)stepIn2          },
//  {"stepIn",            "(II)J",                                    (void*)stepIn3          },
    {"stepOut",           "(J)V",                                     (void*)stepOut          },
    {"message",           "(ILjava/lang/String;J)V",                  (void*)message1         },
//  {"message",           "(IIJ)V",                                   (void*)message2         },
    {"message",           "(ILjava/lang/String;Ljava/lang/String;)V", (void*)message3         },
};

// JNIメソッド配列
static JNINativeMethodEx sWebSocketMethods[] =
{
    {"open",  "(Ljava/lang/String;)V",    (void*)ws_open       },
    {"close", "()V",                      (void*)ws_close      },
    {"send",  "(Ljava/lang/String;)V",    (void*)ws_sendText   },
    {"send",  "(Ljava/nio/ByteBuffer;)V", (void*)ws_sendBinary },
};

/*!
 *  \brief  Java Native Interface OnLoad
 */
extern "C" jint slog_JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JVM = vm;
    JNIEnv* env = getJNIEnv();

    if (env == nullptr)
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
