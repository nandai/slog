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
 *  \file   main.cpp
 *  \brief  シーケンスログサービス (JNI)
 *  \author Copyright 2011-2013 printf.jp
 */
#include "SequenceLogServiceMain.h"
#include "SequenceLogService.h"

#include "slog/Util.h"

using namespace slog;

//f defined(__ANDROID__)
#if defined(__ANDROID__) && !defined(__EXEC__)
#include "slog/JavaString.h"

static jfieldID s_refer;

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
 * Class:     jp_printf_slog_service_App
 * Method:    create
 * Signature: ()V
 */
static void JNICALL create(JNIEnv* env, jobject thiz)
{
    jclass clazz = env->GetObjectClass(thiz);
    s_refer = env->GetFieldID(clazz, "mRefer", "I");

    SequenceLogServiceMain* serviceMain = new SequenceLogServiceMain;
    env->SetIntField(thiz, s_refer, (int)serviceMain);
}

/*
 * Class:     jp_printf_slog_service_App
 * Method:    start
 * Signature: (Ljava/lang/String;Ljava/lang/String;IIZ)V
 */
static void JNICALL start(JNIEnv* env, jobject thiz,
    jstring aSharedMemoryPathName,
    jstring aLogOutputDir,
    jint maxFileSize,
    jint maxFileCount,
    jboolean rootAlways)
{
    JavaString sharedMemoryPathName(env, aSharedMemoryPathName);
    JavaString logOutputDir(        env, aLogOutputDir);

    SequenceLogServiceMain* serviceMain = (SequenceLogServiceMain*)env->GetIntField(thiz, s_refer);

    serviceMain->setSharedMemoryPathName(sharedMemoryPathName);
//  serviceMain->setServiceListener(this);
    serviceMain->setLogFolderName(logOutputDir);
    serviceMain->setMaxFileSize(maxFileSize);
    serviceMain->setMaxFileCount(maxFileCount);
    serviceMain->setRootAlways(rootAlways);
//    serviceMain->setWebServerPort(webServerPort);
//    serviceMain->setSequenceLogServer(sequenceLogServerIp, sequenceLogServerPort);
    serviceMain->start();
}

/*
 * Class:     jp_printf_slog_service_App
 * Method:    stop
 * Signature: ()V
 */
static void JNICALL stop(JNIEnv* env, jobject thiz)
{
    SequenceLogServiceMain* serviceMain = (SequenceLogServiceMain*)env->GetIntField(thiz, s_refer);
//  TRACE("    serviceMain=0x%08X\n", serviceMain);

    Util::stopThread(serviceMain, SERVICE_PORT);

//  env->SetIntField(thiz, s_refer, 0);
}

/*
 * Class:     jp_printf_slog_service_App
 * Method:    canStop
 * Signature: ()Z
 */
static jboolean JNICALL canStop(JNIEnv* env, jobject thiz)
{
    SequenceLogServiceMain* serviceMain = (SequenceLogServiceMain*)env->GetIntField(thiz, s_refer);
    SequenceLogServiceManager* sum = serviceMain->getSequenceLogServiceManager();

    for (SequenceLogServiceManager::iterator i = sum->begin(); i != sum->end(); i++)
    {
        SequenceLogService* service = *i;

        if (service->isAlive())
            return JNI_FALSE;
    }

    return JNI_TRUE;
}

/*
 * Class:     jp_printf_slog_service_App
 * Method:    connectSequenceLogPrint
 * Signature: (Ljava/lang/String;)Z
 */
//static jboolean JNICALL connectSequenceLogPrint(JNIEnv* env, jobject thiz, jstring aIpAddress)
//{
//    JavaString ipAddress(env, aIpAddress);
//
//    SequenceLogServiceMain* serviceMain = (SequenceLogServiceMain*)env->GetIntField(thiz, s_refer);
//    serviceMain->connectSequenceLogPrint(ipAddress);
//
//    return (serviceMain->isConnectSequenceLogPrint() ? JNI_TRUE : JNI_FALSE);
//}

/*
 * Class:     jp_printf_slog_service_App
 * Method:    disconnectSequenceLogPrint
 * Signature: ()V
 */
//static void JNICALL disconnectSequenceLogPrint(JNIEnv* env, jobject thiz)
//{
//    SequenceLogServiceMain* serviceMain = (SequenceLogServiceMain*)env->GetIntField(thiz, s_refer);
//    serviceMain->disconnectSequenceLogPrint();
//}

// JNIメソッド配列
static JNINativeMethod sMethods[] =
{
    {"create",                     "()V",                                        (void*)create                    },
    {"start",                      "(Ljava/lang/String;Ljava/lang/String;IIZ)V", (void*)start                     },
    {"stop",                       "()V",                                        (void*)stop                      },
    {"canStop",                    "()Z",                                        (void*)canStop                   },
//  {"connectSequenceLogPrint",    "(Ljava/lang/String;)Z",                      (void*)connectSequenceLogPrint   },
//  {"disconnectSequenceLogPrint", "()V",                                        (void*)disconnectSequenceLogPrint},
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

    if (registerNatives(env, "net/log_tools/slog/service/App", sMethods, sizeof(sMethods) / sizeof(sMethods[0])) == false)
        return -1;

    return version;
}

#else // defined(__ANDROID__) && !defined(__EXEC__)
#include "slog/String.h"
#include "slog/Tokenizer.h"
#include "slog/DateTimeFormat.h"
#include "slog/FileInfo.h"

#if defined(__unix__)
#include <pwd.h>
#include <grp.h>
#endif

#define VERSION "ver.1.2.3"

class Application : public SequenceLogServiceThreadListener
{
public:     void main(int argc, char** argv);

            virtual void onInitialized(   Thread* thread);
            virtual void onTerminated(    Thread* thread);
            virtual void onLogFileChanged(Thread* thread);
            virtual void onUpdateLog(const Buffer* text);
};

/*!
 *  \brief  アプリケーションメイン
 */
void Application::main(int argc, char** argv)
{
    // コンフィグファイル名取得
    char defaultConf[] = "/etc/slog.conf";
    char* conf = defaultConf;

    if (argc > 1)
    {
        if (strcmp(argv[1], "-f") != 0)
        {
            printf("invalid option: '%s'\n", argv[1]);
            return;
        }

        if (argc == 2)
        {
            printf("option requires an argument: '%s'\n", argv[1]);
            return;
        }

        conf = argv[2];
    }

    // 初期値
    String sharedMemoryDir = "/tmp";
    int32_t sharedMemoryItemCount = 100;
//  String printIp = "127.0.0.1";
    String logOutputDir = "/var/log/slog";
    uint32_t size = 0;
    int32_t count = 0;
    bool rootAlways = true;
    uint16_t webServerPort = 8080;
    String sequenceLogServerIp = "127.0.0.1";
    uint16_t sequenceLogServerPort = 8081;
    String user;
    String group;

    // コンフィグファイル読み込み
    File file;
    file.open(PointerString(conf), File::READ);

    String str;
    String fmt1 = "[key] [value1] [value2]";
    Tokenizer tokenizer(fmt1);

    while (file.read(&str))
    {
        tokenizer.exec(str);

        const CoreString& key = tokenizer.getValue("key");
        const Variant& value1 = tokenizer.getValue("value1");

        if (key == "SHARED_MEMORY_DIR")
            sharedMemoryDir.copy(value1);

        if (key == "SHARED_MEMORY_ITEM_COUNT")
            sharedMemoryItemCount = value1;

//      if (key == "LOG_PRINT_IP")
//          printIp.copy(value1);

        if (key == "LOG_OUTPUT_DIR")
            logOutputDir.copy(value1);

        if (key == "MAX_FILE_SIZE")
        {
            const CoreString& value2 = tokenizer.getValue("value2");
            size = value1;

            if (value2 == "KB")
                size *= 1024;

            if (value2 == "MB")
                size *= (1024 * 1024);
        }

        if (key == "MAX_FILE_COUNT")
            count = value1;

//      if (key == "ROOT_ALWAYS")
//          rootAlways = (value1 == "true");

        if (key == "WEB_SERVER_PORT")
            webServerPort = value1;

        if (key == "SEQUENCE_LOG_SERVER_IP")
            sequenceLogServerIp.copy(value1);

        if (key == "SEQUENCE_LOG_SERVER_PORT")
            sequenceLogServerPort = value1;

        if (key == "USER")
            user.copy(value1);

        if (key == "GROUP")
            group.copy(value1);
    }

    // サービス起動
    SequenceLogServiceMain serviceMain;

#if defined(__unix__)
    serviceMain.setSharedMemoryPathName(sharedMemoryDir);
#endif

    serviceMain.setSharedMemoryItemCount(sharedMemoryItemCount);
    serviceMain.setServiceListener(this);
    serviceMain.setLogFolderName(logOutputDir);
    serviceMain.setMaxFileSize(size);
    serviceMain.setMaxFileCount(count);
    serviceMain.setRootAlways(rootAlways);
    serviceMain.setWebServerPort(webServerPort);
    serviceMain.setSequenceLogServer(sequenceLogServerIp, sequenceLogServerPort);
    serviceMain.start();

#if defined(__unix__)
    struct group* gr =  (group.getLength() ? getgrnam(group.getBuffer()) : NULL);
    struct passwd* pw = (user. getLength() ? getpwnam(user. getBuffer()) : NULL);

    if (gr)
        setgid(gr->gr_gid);

    if (pw)
        setuid(pw->pw_uid);
#endif

    Thread::sleep(100);
    serviceMain.join();
}

/*!
 *  \brief  シーケンスログサービススレッド初期化完了通知
 */
void Application::onInitialized(Thread* thread)
{
    TRACE("[S] Application::onInitialized()\n", 0);

#if defined(__ANDROID__)
    SequenceLogService* service = (SequenceLogService*)thread;
#else
    SequenceLogService* service = dynamic_cast<SequenceLogService*>(thread);
#endif

    FileInfo* fileInfo = service->getFileInfo();
    TRACE("fileInfo: 0x%08X\n", fileInfo);

    if (fileInfo)
    {
        DateTime dateTime = fileInfo->getCreationTime();
        dateTime.toLocal();

        FixedString<DateTimeFormat::DATE_TIME_MS_LEN> str;
        DateTimeFormat::toString(&str, dateTime, DateTimeFormat::DATE_TIME);

        noticeLog("start %s %s\n", str.getBuffer(), fileInfo->getCanonicalPath().getBuffer());
    }

    TRACE("[E] Application::onInitialized()\n", 0);
}

/*!
 *  \brief  シーケンスログサービススレッド終了通知
 */
void Application::onTerminated(Thread* thread)
{
#if defined(__ANDROID__)
    SequenceLogService* service = (SequenceLogService*)thread;
#else
    SequenceLogService* service = dynamic_cast<SequenceLogService*>(thread);
#endif
    FileInfo* fileInfo = service->getFileInfo();
    DateTime dateTime = fileInfo->getLastWriteTime();

    if (dateTime.getValue())
    {
        dateTime.toLocal();

        FixedString<DateTimeFormat::DATE_TIME_MS_LEN> str;
        DateTimeFormat::toString(&str, dateTime, DateTimeFormat::DATE_TIME);

        noticeLog("end   %s %s\n", str.getBuffer(), fileInfo->getCanonicalPath().getBuffer());
    }
    else
    {
        noticeLog("%s %s\n", fileInfo->getMessage().getBuffer(), fileInfo->getCanonicalPath().getBuffer());
    }
}

/*!
 *  \brief  シーケンスログファイル変更通知
 */
void Application::onLogFileChanged(Thread* thread)
{
    onInitialized(thread);
}

/*!
 *  \brief	シーケンスログ更新通知
 */
void Application::onUpdateLog(const Buffer* text)
{
    const char* p = text->getBuffer();

    switch (*p)
    {
    case 'd':
        printf("\x1B[32;49;0m");
        break;

    case 'i':
        printf("\x1B[37;49;1m");
        break;

    case 'w':
        printf("\x1B[33;49;1m");
        break;

    case 'e':
        printf("\x1B[31;49;1m");
        break;
    }

    printf("%s", p + 1);
}

#if defined(__unix__)
#include <signal.h>

/*!
 *  \brief  シグナル受信
 */
static void onSignal(int sig)
{
    if (sig == SIGINT)
        puts("");

    SequenceLogServiceMain* serviceMain = SequenceLogServiceMain::getInstance();
    Util::stopThread(serviceMain, SERVICE_PORT);
}
#endif

/*!
 *  \brief  メイン
 */
int main(int argc, char** argv)
{
    noticeLog("SequenceLogService " VERSION " is starting");

#if defined(__unix__)
    signal(SIGINT,  onSignal);
//  signal(SIGTERM, onSignal);
#endif

    Socket::startup();

    try
    {
        Application app;
        app.main(argc, argv);
    }
    catch (Exception e)
    {
        printf("%s\n", e.getMessage());
    }

    Socket::cleanup();
    return 0;
}

/*!
 *  \brief  シーケンスログファイル名取得
 *
 *  \note   libslog.soをリンクしているので、この関数を定義する必要がある。ただし呼ばれることはない。
 */
extern "C" const char* getSequenceLogFileName()
{
    return NULL;
}

#endif // defined(__ANDROID__) && !defined(__EXEC__)
