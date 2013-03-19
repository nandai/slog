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

public class App extends android.app.Application
{
    private int     mRefer = 0;
    private boolean mServiceRunning = false;

    public String   mSharedMemoryPathName;  // 共有メモリパス
    public String   mLogOutputDir;          // ログ出力ディレクトリ
    public int      mMaxFileSize;           // 最大ファイルサイズ
    public String   mMaxFileSizeUnit;       // 最大ファイルサイズ単位
    public int      mMaxFileCount;          // 最大ファイル数
    public int      mWebServerPort;         // Web Server ポート
    public String   mSequenceLogServerIp;   // Sequence Log Server IP
    public int      mSequenceLogServerPort; // Sequence Log Server ポート

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

    // Sequence Log Service の本体生成
    private native void create();

    // Sequence Log Service 開始
    public  void start()
    {
        start(
            mSharedMemoryPathName,
            mLogOutputDir,
            mMaxFileSize * (mMaxFileSizeUnit.equals("KB") ? 1024 : 1024 * 1024),
            mMaxFileCount,
            false);
    }

    // Sequence Log Service 開始
    private native void start(
        String sharedMemoryPathName,
        String logOutputDir,
        int maxFileSize,
        int maxFileCount,
        boolean rootAlways);

    // Sequence Log Service 停止
    public  native void stop();
    public  native boolean canStop();

    // シーケンスログプリント関連
//  public  native boolean connectSequenceLogPrint(String ipAddress);
//  public  native void disconnectSequenceLogPrint();
}
