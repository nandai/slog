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
import java.io.IOException;

public class App extends android.app.Application
{
    private int     mRefer = 0;
    private boolean mServiceRunning = false;

    public String   mSharedMemoryPathName;  // 共有メモリパス
    public String   mLogOutputDir;          // ログ出力ディレクトリ
    public int      mMaxFileSize;           // 最大ファイルサイズ
    public String   mMaxFileSizeUnit;       // 最大ファイルサイズ単位
    public int      mMaxFileCount;          // 最大ファイル数

    private Process             mSu = null;
    private DataInputStream     mInputStream = null;
    private DataOutputStream    mOutputStream = null;

    static
    {
        System.loadLibrary("slogsvc");
    }

    @Override
    public void onCreate() 
    {
        super.onCreate();
        create();
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

    // Sequence Log Service の本体生成
    private native void create();

    // Sequence Log Service 開始
    public  native void start();

    // Sequence Log Service 停止
    public  native void stop();
    public  native boolean canStop();

    // 設定反映
    public void updateSettings()
    {
        setSettings(
            mSharedMemoryPathName,
            mLogOutputDir,
            mMaxFileSize * (mMaxFileSizeUnit.equals("KB") ? 1024 : 1024 * 1024),
            mMaxFileCount);
    }

    // Sequence Log Service 開始
    private native void setSettings(
        String sharedMemoryPathName,
        String logOutputDir,
        int maxFileSize,
        int maxFileCount);

    /**
     * Sequence Log Service Web Server ポート設定
     * 
     * @param   port    Sequence Log Service Web Server のポート
     */
    public  native void setWebServerPort(int port);

    /**
     * Sequence Log Server のIPとポートを設定
     * 
     * @param   ip      Sequence Log Server IP
     * @param   port    Sequence Log Server のポート
     */
    public  native void setSequenceLogServer(String ip, int port);
}
