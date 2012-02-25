/*
 * Copyright (C) 2011 log-tools.net
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
package net.log_tools.slog.service;

public class App extends android.app.Application
{
    private int     mRefer = 0;
    private boolean mServiceRunning = false;
    private boolean mConnecting = false;

    public String   mSharedMemoryPathName;
    public String   mLogOutputDir;
    public int      mMaxFileSize;
    public String   mMaxFileSizeUnit;
    public int      mMaxFileCount;
    public boolean  mRootAlways;

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

    public boolean isRunning()
    {
        return mServiceRunning;
    }

    public void running(boolean running)
    {
        mServiceRunning = running;
    }

    public boolean isConnecting()
    {
        return mConnecting;
    }
    
    public void connecting(boolean connecting)
    {
        mConnecting = connecting;
    }

    private native void create();

    // シーケンスログサービス関連
    public  void start()
    {
        start(
            mSharedMemoryPathName,
            mLogOutputDir,
            mMaxFileSize * (mMaxFileSizeUnit.equals("KB") ? 1024 : 1024 * 1024),
            mMaxFileCount,
            mRootAlways);
    }

    private native void start(
        String sharedMemoryPathName,
        String logOutputDir,
        int maxFileSize,
        int maxFileCount,
        boolean rootAlways);

    public  native void stop();
    public  native boolean canStop();

    // シーケンスログプリント関連
    public  native boolean connectSequenceLogPrint(String ipAddress);
    public  native void disconnectSequenceLogPrint();
}
