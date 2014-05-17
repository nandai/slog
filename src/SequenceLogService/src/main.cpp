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
#include "SequenceLogServiceDB.h"

#include "slog/Util.h"
#include "slog/PointerString.h"

#if defined(__unix__)
    #include <stdio.h>
    #include <string.h>
    #include <errno.h>
    #include <unistd.h>
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

#define VERSION "ver.1.2.8"

/*!
 *  \brief  アクセス可能なディレクトリかどうか
 */
static bool isDirectoryPermissionAllow(const CoreString& dirName)
{
    String path;
    path.format("%s%c", dirName.getBuffer(), PATH_DELIMITER);

    try
    {
        FileInfo fileInfo(&path);
        fileInfo.mkdir();
    }
    catch (Exception)
    {
        return false;
    }

    return true;
}

/*!
 *  \brief  アプリケーションクラス
 */
class Application : public ThreadListener, public SequenceLogServiceListener
{
public:     void main(int argc, char** argv);

            virtual void onThreadInitialized(Thread* thread) override;
            virtual void onThreadTerminated( Thread* thread) override;
            virtual void onLogFileChanged(   Thread* thread, const CoreString* fileName, int32_t userId) override;
            virtual void onUpdateLog(const Buffer* text, int32_t userId) override;
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
    String logOutputDir = "/var/log/slog";
    uint32_t size = 0;
    int32_t count = 0;
    uint16_t webServerPort = 8080;
    uint16_t webServerPortSSL = 8443;
    uint16_t sequenceLogServerPort = 8081;
    String certificate;
    String privateKey;
    String user;
    String group;

    // コンフィグファイル読み込み
    File file;
    file.open(conf, File::READ);

    String str;
    String fmt1 = "[key] [value1] [value2]";
    Tokenizer tokenizer(&fmt1);

    while (file.read(&str))
    {
        tokenizer.exec(&str);

        const CoreString* key = tokenizer.getValue("key");
        const Variant& value1 = tokenizer.getValue("value1");

        if (key->equals("LOG_OUTPUT_DIR"))
            logOutputDir.copy(value1);

        if (key->equals("MAX_FILE_SIZE"))
        {
            const CoreString* value2 = tokenizer.getValue("value2");
            size = value1;

            if (value2->equals("KB"))
                size *= 1024;

            if (value2->equals("MB"))
                size *= (1024 * 1024);
        }

        if (key->equals("MAX_FILE_COUNT"))
            count = value1;

        if (key->equals("WEB_SERVER_PORT"))
            webServerPort = value1;

        if (key->equals("WEB_SERVER_PORT_SSL"))
            webServerPortSSL = value1;

        if (key->equals("SEQUENCE_LOG_SERVER_PORT"))
            sequenceLogServerPort = value1;

        if (key->equals("CERTIFICATE"))
            certificate.copy(value1);

        if (key->equals("PRIVATE_KEY"))
            privateKey.copy(value1);

        if (key->equals("USER"))
            user.copy(value1);

        if (key->equals("GROUP"))
            group.copy(value1);
    }

    // ユーザーとグループを変更する
    if (changeGroupAndUser(group, user) == false)
        return;

    // サービス起動
    SequenceLogServiceMain serviceMain;
    int32_t check = 0x00;

    if (isDirectoryPermissionAllow(logOutputDir) == false)
        check |= 0x02;

    if (check != 0x00)
    {
        printf("%03X permission denied.\n", check);
        fflush(stdout); // 必須！flushしないと受け手のプロセスで標準出力を読み込めない
    }
    else
    {
        SequenceLogServiceDB db;
        db.init();

//      serviceMain.setListener(this);
        serviceMain.setLogFolderName(&logOutputDir);
        serviceMain.setMaxFileSize(size);
        serviceMain.setMaxFileCount(count);
        serviceMain.setWebServerPort(false, webServerPort);
        serviceMain.setWebServerPort(true,  webServerPortSSL);
        serviceMain.setSSLFileName(&certificate, &privateKey);
        serviceMain.setSequenceLogServerPort(sequenceLogServerPort);
        serviceMain.start();

        printf("%03X OK\n", check);
        fflush(stdout); // 必須！flushしないと受け手のプロセスで標準出力を読み込めない
    }

#if defined(__ANDROID__)
    getchar();
    serviceMain.interrupt();
    serviceMain.join();
#else
    Thread::sleep(100);
    serviceMain.join();
#endif
}

/*!
 *  \brief  シーケンスログサービススレッド初期化完了通知
 */
void Application::onThreadInitialized(Thread* thread)
{
    // SequenceLogService取得
    SequenceLogService*  service = dynamic_cast<SequenceLogService*>(thread);

    if (service == NULL)
        return;

    // FileInfo取得
    FileInfo* fileInfo = service->getFileInfo();

    if (fileInfo == NULL)
        return;

    DateTime dateTime = fileInfo->getCreationTime();
    dateTime.toLocal();

    FixedString<(int32_t)DateTimeFormat::Length::YYYYMMDDHHMISS> str;
    DateTimeFormat::toString(&str, dateTime, DateTimeFormat::Format::YYYYMMDDHHMISS);

    noticeLog("start %s %s\n", str.getBuffer(), fileInfo->getCanonicalPath()->getBuffer());
}

/*!
 *  \brief  シーケンスログサービススレッド終了通知
 */
void Application::onThreadTerminated(Thread* thread)
{
    // SequenceLogService取得
    SequenceLogService* service = dynamic_cast<SequenceLogService*>(thread);

    if (service == NULL)
        return;

    // FileInfo取得
    FileInfo* fileInfo = service->getFileInfo();

    if (fileInfo == NULL)
        return;

    DateTime dateTime = fileInfo->getLastWriteTime();

    if (dateTime.getValue())
    {
        dateTime.toLocal();

        FixedString<(int32_t)DateTimeFormat::Length::YYYYMMDDHHMISS> str;
        DateTimeFormat::toString(&str, dateTime, DateTimeFormat::Format::YYYYMMDDHHMISS);

        noticeLog("end   %s %s\n", str.getBuffer(), fileInfo->getCanonicalPath()->getBuffer());
    }
    else
    {
        noticeLog("%s %s\n", fileInfo->getMessage()->getBuffer(), fileInfo->getCanonicalPath()->getBuffer());
    }
}

/*!
 *  \brief  シーケンスログファイル変更通知
 */
void Application::onLogFileChanged(Thread* thread, const CoreString* fileName, int32_t userId)
{
    onThreadInitialized(thread);
}

/*!
 *  \brief  シーケンスログ更新通知
 */
void Application::onUpdateLog(const Buffer* text, int32_t userId)
{
    const char* p = text->getBuffer();

    switch (*p)
    {
    case 'd':
        printf("\x1B[32;49;1m");
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
    serviceMain->interrupt();
}
#endif

/*!
 *  \brief  メイン
 */
int main(int argc, char** argv)
{
    noticeLog("SequenceLogService " VERSION " is starting");

    loadSequenceLogConfig("~/SequenceLogService.log.config");
//  SLOG("main.cpp", "main");

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
        noticeLog("%s\n", e.getMessage());
    }

    Socket::cleanup();

    printf("EXIT\n");
    return 0;
}
