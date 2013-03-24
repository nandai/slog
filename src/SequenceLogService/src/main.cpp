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

#if defined(__unix__)
    #include <pwd.h>
    #include <grp.h>
#endif

using namespace slog;

/*!
 *  \brief  グループとユーザーを変更する
 */
static bool changeGroupAndUser(const CoreString& aGroup, const CoreString& user)
{
#if defined(__unix__)
    // 必ず以下の順序で変更する
    //
    // 1.グループ変更
    // 2.ユーザー変更

    // 1.グループ変更
    if (0 < aGroup.getLength())
    {
        struct group* gr = getgrnam(aGroup.getBuffer());

        if (gr == NULL || setgid(gr->gr_gid) != 0)
        {
            String err;
            err.format("change group '%s' ... %s\n",
                aGroup.getBuffer(),
                gr ? strerror(errno) : "Not found");

            const char* p = err.getBuffer();
            noticeLog(p);

            return false;
        }
    }

    // 2.ユーザー変更
    if (0 < user.getLength())
    {
        struct passwd* pw = (user. getLength() ? getpwnam(user. getBuffer()) : NULL);

        if (pw == NULL || setuid(pw->pw_uid) != 0)
        {
            String err;
            err.format("change user '%s' ... %s\n",
                user.getBuffer(),
                pw ? strerror(errno) : "Not found");

            const char* p = err.getBuffer();
            noticeLog(p);

            return false;
        }
    }
#endif

    return true;
}

#include "slog/String.h"
#include "slog/Tokenizer.h"
#include "slog/DateTimeFormat.h"
#include "slog/FileInfo.h"

#define VERSION "ver.1.2.4"

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
    String logOutputDir = "/var/log/slog";
    uint32_t size = 0;
    int32_t count = 0;
    bool outputScreen = true;
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

        if (key == "OUTPUT_SCREEN")
            outputScreen = (value1 == "true");

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

    // ユーザーとグループを変更する
    if (changeGroupAndUser(group, user) == false)
        return;

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
    serviceMain.setOutputScreen(outputScreen);
    serviceMain.setWebServerPort(webServerPort);
    serviceMain.setSequenceLogServer(sequenceLogServerIp, sequenceLogServerPort);
    serviceMain.start();

#if defined(__ANDROID__)
    getchar();
    Util::stopThread(&serviceMain, SERVICE_PORT);
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
 *  \brief  シーケンスログ更新通知
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

    printf("EXIT\n");
    return 0;
}
