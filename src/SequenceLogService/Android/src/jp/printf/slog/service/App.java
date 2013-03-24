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

    private Process             mExec = null;           // 実行プロセス
    private String              mExecPath;              // 実行ファイルパス
    private String              mConfigPath;            // 設定ファイルパス

    private Process             mSu = null;             // suプロセス
    private DataInputStream     mInputStream = null;
    private DataOutputStream    mOutputStream = null;

    @Override
    public void onCreate() 
    {
        super.onCreate();

        mExecPath =   getFileStreamPath("slogsvc").  getAbsolutePath();
        mConfigPath = getFileStreamPath("slog.conf").getAbsolutePath();

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
        if (mSu != null && on)
        {
            // 既にスーパーユーザー
            return true;
        }

        if (mSu == null && on == false)
        {
            // 既に一般ユーザー
            return true;
        }

        boolean result = true;

        try
        {
            if (on)
            {
                result = false;

                mSu = Runtime.getRuntime().exec("su");
                mInputStream =  new DataInputStream( mSu.getInputStream());
                mOutputStream = new DataOutputStream(mSu.getOutputStream());

                mOutputStream.writeBytes("whoami\n");
                mOutputStream.flush();

                byte[] buffer = new byte[64];
                int size = mInputStream.read(buffer);

                if (0 < size)
                {
                    String res = new String(buffer, 0, size - 1);

                    if (res.equals("root") == true)
                    {
                        // スーパーユーザー化成功
                        return true;
                    }
                }
            }
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }

        // OutputStream後始末
        try
        {
            if (mOutputStream != null)
            {
                mOutputStream.writeBytes("exit\n");
                mOutputStream.flush();
                mOutputStream.close();
            }
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
        finally
        {
            mOutputStream = null;
        }

        // InputStream後始末
        try
        {
            if (mInputStream != null)
            {
                mInputStream.close();
            }
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
        finally
        {
            mInputStream = null;
        }

        // suプロセス後始末
        if (mSu != null)
        {
            mSu.destroy();
            mSu = null;
        }

        return result;
    }

    // 使えるディレクトリかどうか
    public static boolean canUsableDirectory(String path)
    {
        File dir = new File(path);

        if (dir.exists() == false)
        {
            if (dir.mkdirs() == false)
                return false;
        }

        try
        {
            File file = new File(path + "/SequenceLogServiceDirectoryCheck");
            file.createNewFile();
            file.delete();
        }
        catch (IOException e)
        {
            e.printStackTrace();
            return false;
        }

        return true;
    }

    public void start()
    {
        try
        {
            // 設定ファイル生成
            File file = new File(mConfigPath);
            FileWriter writer = new FileWriter(file);

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
            writer.close();

            // 実行
            if (mSu == null)
            {
                String commands[] = {mExecPath, "-f", mConfigPath};
                mExec = Runtime.getRuntime().exec(commands);
            }
            else
            {
                mOutputStream.writeBytes(mExecPath + " -f " + mConfigPath + "\n");
                mOutputStream.flush();
            }
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
    }

    // Sequence Log Service 停止
    public void stop()
    {
        try
        {
            if (mSu == null)
            {
                DataOutputStream outputStream = new DataOutputStream(mExec.getOutputStream());
                outputStream.writeBytes("\n");
                outputStream.flush();
            }
            else
            {
                mOutputStream.writeBytes("\n");
                mOutputStream.flush();
            }
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
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
