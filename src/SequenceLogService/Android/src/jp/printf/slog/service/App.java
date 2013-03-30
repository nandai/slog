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
package jp.printf.slog.service;

import java.io.Closeable;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.res.AssetManager;
import android.util.Log;

@SuppressLint("DefaultLocale")
public class App extends android.app.Application
{
    private boolean mServiceRunning = false;

    public String   mSharedMemoryPathName;  // 共有メモリパス
    public String   mLogOutputDir;          // ログ出力ディレクトリ
    public int      mMaxFileSize;           // 最大ファイルサイズ
    public String   mMaxFileSizeUnit;       // 最大ファイルサイズ単位
    public int      mMaxFileCount;          // 最大ファイル数
    public int      mWebServerPort;         // Webサーバーポート
    public String   mSequenceLogServerIp;   // Sequence Log Server IP
    public int      mSequenceLogServerPort; // Sequence Log Server ポート

    private String              mExecPath;              // 実行ファイルパス
    private String              mConfigPath;            // 設定ファイルパス

    private boolean             mSuperUser = false;
    private Process             mShell = null;          // shプロセス
    private DataInputStream     mInputStream = null;    // 標準入力
    private DataOutputStream    mOutputStream = null;   // 標準出力

    @Override
    public void onCreate() 
    {
        super.onCreate();

        mExecPath =   getFileStreamPath("slogsvc").  getAbsolutePath();
        mConfigPath = getFileStreamPath("slog.conf").getAbsolutePath();

        try
        {
            mShell = Runtime.getRuntime().exec("sh");
            mInputStream =  new DataInputStream( mShell.getInputStream());
            mOutputStream = new DataOutputStream(mShell.getOutputStream());
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }

        install();
    }

    // Sequence Log Service が開始されているかどうか
    public boolean isRunning()
    {
        return mServiceRunning;
    }

    // Sequence Log Service の動作状態設定
    public void running(boolean running)
    {
        mServiceRunning = running;
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
                byte[] buffer = new byte[64];
                int size = mInputStream.read(buffer);

                if (0 < size)
                    res = new String(buffer, 0, size - 1);
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
        FileWriter writer = null;
        int result = -1;
        String response = "";

        try
        {
            // 設定ファイル生成
            File file = new File(mConfigPath);
            writer = new FileWriter(file);

            String config = String.format(
                "SHARED_MEMORY_DIR        %s\n" +
                "SHARED_MEMORY_ITEM_COUNT 300\n" +
                "LOG_OUTPUT_DIR           %s\n" +
                "MAX_FILE_SIZE            %d %s\n" +
                "MAX_FILE_COUNT           %d\n" +
                "OUTPUT_SCREEN            false\n" +
                "WEB_SERVER_PORT          %d\n" +
                "SEQUENCE_LOG_SERVER_IP   %s\n" +
                "SEQUENCE_LOG_SERVER_PORT %d\n",

                mSharedMemoryPathName,
                mLogOutputDir,
                mMaxFileSize, mMaxFileSizeUnit,
                mMaxFileCount,
                mWebServerPort,
                mSequenceLogServerIp,
                mSequenceLogServerPort);

            writer.write(config);
            close(writer);
            writer = null;

            // 実行
            writeStream(mExecPath + " -f " + mConfigPath + "\n");

            response = readStream();
            result = Integer.parseInt(response.substring(0, 3));
//          Log.i("slog", response);

            if (result == 0)
            {
                running(true);
            }
            else
            {
                stop();
            }
        }
        catch (Exception e)
        {
            Log.e("slog", response);
            e.printStackTrace();
        }
        finally
        {
            close(writer);
        }

        return result;
    }

    // Sequence Log Service 停止
    public void stop()
    {
        writeStream("\n");
        String res = readStream();

        if (res.equals("EXIT") == true)
        {
        }

        running(false);
    }

    /**
     * インストール
     */
    public void install()
    {
        try
        {
            // Sequence Log Service の実体をassetsからfiles/にコピーする
            AssetManager am = getResources().getAssets();

            // 読み込み
            InputStream is = am.open("bin/slogsvc");
            int size =  is.available();
            byte[] buffer = new byte[size];

            is.read(buffer);
            is.close();

            // 書き込み
            FileOutputStream os = openFileOutput("slogsvc", Context.MODE_PRIVATE);
            os.write(buffer);
            os.flush();
            os.close();

            // 実行権限付与
            String commands[] = {"chmod", "755", ""};
            commands[2] = mExecPath;

            Runtime.getRuntime().exec(commands);
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
    }
}
