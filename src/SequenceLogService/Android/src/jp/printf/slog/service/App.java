/*
 * Copyright (C) 2011-2014 printf.jp
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
package jp.printf.slog.service;

import java.io.BufferedReader;
import java.io.Closeable;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

import jp.printf.slog.Log;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.res.AssetManager;

@SuppressLint("DefaultLocale")
public class App extends android.app.Application
{
    private final String CLS_NAME = "App";

    public final int SERVICE_STOPPED =  0;
    public final int SERVICE_RUNNING =  1;
    public final int SERVICE_STOPPING = 2;

    /**
     * ステータス
     */
    private int mServiceStatus = SERVICE_STOPPED;

    /**
     * ログ出力ディレクトリ
     */
    public String mLogOutputDir;

    /**
     * 最大ファイルサイズ
     */
    public int mMaxFileSize;

    /**
     * 最大ファイルサイズ単位
     */
    public String mMaxFileSizeUnit;

    /**
     * 最大ファイル数
     */
    public int mMaxFileCount;

    /**
     * Webサーバーポート
     */
    public int mWebServerPort;

    /**
     * Webサーバーポート (SSL)
     */
    public int mWebServerPortSSL;

    /**
     * Sequence Log Server ポート
     */
    public int mSequenceLogServerPort;

    /**
     * 実行ファイルパス
     */
    private String mExecPath;

    /**
     * 設定ファイルパス
     */
    private String mConfigPath;

    /**
     * スーパーユーザーモードフラグ
     */
    private boolean mSuperUser = false;

    /**
     * shプロセス
     */
    private Process mShell = null;

    /**
     * 標準入力
     */
    private BufferedReader mInputStream = null;

    /**
     * 標準出力
     */
    private DataOutputStream mOutputStream = null;

    /**
     * 標準入力読み出しスレッド
     */
    private InputStreamThread mInputStreamThread = null;

    /**
     * onCreate
     */
    @Override
    public void onCreate() 
    {
        {
            System.loadLibrary("slog");
            Log.setFileName("SequenceLogServiceForAndroid.slog");
            Log.setServiceAddress("ws://192.168.0.2:8080");     // 自分自身以外の端末を指定する
            Log.enableOutput(false);
        }

        long TAG = Log.stepIn(CLS_NAME, "onCreate");
        super.onCreate();

        mExecPath =   getFileStreamPath("slogsvc").  getAbsolutePath();
        mConfigPath = getFileStreamPath("slog.conf").getAbsolutePath();

        try
        {
            mShell = Runtime.getRuntime().exec("sh");
            mInputStream =  new BufferedReader(new InputStreamReader(mShell.getInputStream()));
            mOutputStream = new DataOutputStream(                    mShell.getOutputStream());
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }

        install("bin/slogsvc",       "slogsvc",       "755");
        install("web/index.html",    "index.html",    "644");
        install("web/notfound.html", "notfound.html", "644");
        install("web/login.html",    "login.html",    "644");
        install("web/login.js",      "login.js",      "644");
        install("web/login.pcss",    "login.pcss",    "644");
        install("web/account.html",  "account.html",  "644");
        install("web/wait.gif",      "wait.gif",      "644");

        install("web/SequenceLogService.pcss", "SequenceLogService.pcss", "644");
        install("web/SequenceLogService.js",   "SequenceLogService.js",   "644");

        Log.stepOut(TAG);
    }

    // Sequence Log Service のステータスを取得
    public int getServiceStatus()
    {
        return mServiceStatus;
    }

    // Sequence Log Service のステータスを設定
    public void setServiceStatus(int status)
    {
        mServiceStatus = status;
    }

    // スーパーユーザー設定
    public boolean setSuperUser(boolean on)
    {
        if (mSuperUser == on)
            return true;

        if (on)
        {
            writeStream("su\n");
            writeStream("whoami\n");

            String res = readStream();
            mSuperUser = res.equals("root");
        }
        else
        {
            writeStream("exit\n");
            mSuperUser = false;
        }

        return mSuperUser;
    }

    // 標準入力から読み込む
    private String readStream()
    {
        String res = "";

        try
        {
            if (mInputStream != null)
            {
                res = mInputStream.readLine();
            }
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }

        return res;
    }

    // 標準出力に書き込む
    private void writeStream(String command)
    {
        try
        {
            if (mOutputStream == null)
                return;

            mOutputStream.writeBytes(command);
            mOutputStream.flush();
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
    }

//// 標準入力と標準出力をクローズする
//    private void closeStreams()
//    {
//        // OutputStreamクローズ
//        close(mOutputStream);
//        mOutputStream = null;
//
//        // InputStreamクローズ
//        close(mInputStream);
//        mInputStream = null;
//    }
//
    // 例外が発生しようがとにかくclose()を呼び出す
    static private void close(Closeable obj)
    {
        try
        {
            if (obj != null)
                obj.close();
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
    }

    public int start()
    {
        long TAG = Log.stepIn(CLS_NAME, "start");
        FileWriter writer = null;
        int result = -1;
        String response = "";

        try
        {
            // 設定ファイル生成
            File file = new File(mConfigPath);
            writer = new FileWriter(file);

            String config = String.format(
                "LOG_OUTPUT_DIR           %s\n" +
                "MAX_FILE_SIZE            %d %s\n" +
                "MAX_FILE_COUNT           %d\n" +
                "OUTPUT_SCREEN            true\n" +
                "WEB_SERVER_PORT          %d\n" +
//              "WEB_SERVER_PORT_SSL      %d\n" +
                "SEQUENCE_LOG_SERVER_PORT %d\n",

                mLogOutputDir,
                mMaxFileSize, mMaxFileSizeUnit,
                mMaxFileCount,

                mWebServerPort,
//              mWebServerPortSSL,
                mSequenceLogServerPort);

            writer.write(config);
            close(writer);
            writer = null;

            // 実行
            writeStream(mExecPath + " -f " + mConfigPath + "\n");

            response = readStream();
            result = Integer.parseInt(response.substring(0, 3));
//          Log.d(TAG, response);

            if (result == 0)
            {
                setServiceStatus(SERVICE_RUNNING);

                mInputStreamThread = new InputStreamThread(mInputStream);
                mInputStreamThread.start();
            }
            else
            {
                stop();
            }
        }
        catch (Exception e)
        {
            Log.e(TAG, response);
            e.printStackTrace();
        }
        finally
        {
            close(writer);
        }

        Log.stepOut(TAG);
        return result;
    }

    // Sequence Log Service 停止
    public void stop()
    {
        long TAG = Log.stepIn(CLS_NAME, "stop");

        writeStream("\n");
        setServiceStatus(SERVICE_STOPPING);

        try
        {
            mInputStreamThread.interrupt();
            mInputStreamThread.join();
        }
        catch (InterruptedException e)
        {
            e.printStackTrace();
        }

        setServiceStatus(SERVICE_STOPPED);
        Log.stepOut(TAG);
    }

    /**
     * インストール
     */
    public void install(String src, String dest, String mode)
    {
        try
        {
            // assets/のファイルをfiles/にコピーする
            AssetManager am = getResources().getAssets();

            // 読み込み
            InputStream is = am.open(src);
            int size =  is.available();
            byte[] buffer = new byte[size];

            is.read(buffer);
            is.close();

            // 書き込み
            FileOutputStream os = openFileOutput(dest, Context.MODE_PRIVATE);
            os.write(buffer);
            os.flush();
            os.close();

            // 実行権限付与
            String commands[] = {"chmod", mode, ""};
            commands[2] = getFileStreamPath(dest).getAbsolutePath();

            Runtime.getRuntime().exec(commands);
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
    }
}

/**
 * 標準入力読み出しスレッド
 */
class InputStreamThread extends Thread
{
    private BufferedReader mInputStream;

    public InputStreamThread(BufferedReader inputStream)
    {
        mInputStream = inputStream;
    }

    public void run()
    {
        String str;

        while (true)
        {
            try
            {
                str = mInputStream.readLine();

                if (isInterrupted())
                {
                    if (str.equals("EXIT"))
                        break;
                }
            }
            catch (IOException e)
            {
                e.printStackTrace();
            }
        }
    }
}
