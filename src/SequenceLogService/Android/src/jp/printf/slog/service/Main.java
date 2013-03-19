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

import android.app.Activity;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.text.SpannableStringBuilder;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
//port android.util.Log;

public class Main extends Activity
{
    private Intent      mIntent;
    
    private Button      mStartStopService;          // サービス開始／終了ボタン
    private EditText    mSharedMemoryPathName;      // 共有メモリ設定エリア
//  private Button      mConnDisconnPrint;          // シーケンスログプリント接続／切断ボタン
//  private EditText    mIPAddress;                 // シーケンスログプリントIPアドレス設定エリア
    private EditText    mLogOutputDir;              // ログ出力ディレクトリ
    private EditText    mMaxFileSize;               // 最大ログファイルサイズ
    private Spinner     mMaxFileSizeUnit;           // 最大ログファイルサイズ単位
    private EditText    mMaxFileCount;              // 最大ログファイル数
    private EditText    mWebServerPort;             // Web Server ポート
    private EditText    mSequenceLogServerIp;       // Sequence Log Server IP
    private EditText    mSequenceLogServerPort;     // Sequence Log Server ポート

    private void setSetting()
    {
        App app = (App)getApplication();
        SpannableStringBuilder sb;

        sb = (SpannableStringBuilder)mSharedMemoryPathName.getText();
        app.mSharedMemoryPathName = sb.toString();
        
        sb = (SpannableStringBuilder)mLogOutputDir.getText();
        app.mLogOutputDir = sb.toString();

        sb = (SpannableStringBuilder)mMaxFileSize.getText();
        app.mMaxFileSize = Integer.parseInt(sb.toString());

        app.mMaxFileSizeUnit = (String)mMaxFileSizeUnit.getSelectedItem();

        sb = (SpannableStringBuilder)mMaxFileCount.getText();
        app.mMaxFileCount = Integer.parseInt(sb.toString());

        sb = (SpannableStringBuilder)mWebServerPort.getText();
        app.mWebServerPort = Integer.parseInt(sb.toString());

        sb = (SpannableStringBuilder)mSequenceLogServerIp.getText();
        app.mSequenceLogServerIp = sb.toString();

        sb = (SpannableStringBuilder)mSequenceLogServerPort.getText();
        app.mSequenceLogServerPort = Integer.parseInt(sb.toString());
    }

    private void start()
    {
        setSetting();

        mStartStopService.setText(getString(R.string.stop_service));
        startService(mIntent);

        Intent activityIntent = new Intent(this, Main.class);
        activityIntent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_NEW_TASK);

        PendingIntent contentIntent = PendingIntent.getActivity(this, 0, activityIntent, Intent.FLAG_ACTIVITY_NEW_TASK);

        Notification notification = new Notification.Builder(this)
            .setSmallIcon(R.drawable.icon)
            .setTicker(getString(R.string.start_service))
            .setWhen(System.currentTimeMillis())
            .setContentTitle(getText(R.string.app_name))
            .setContentText("Started.")
            .setContentIntent(contentIntent)
            .build();

        NotificationManager manager = (NotificationManager)getSystemService(NOTIFICATION_SERVICE);
        manager.notify(0, notification);
    }

    private void stop()
    {
        mStartStopService.setText(getString(R.string.start_service));
        stopService(mIntent);

        NotificationManager manager = (NotificationManager)getSystemService(NOTIFICATION_SERVICE);
        manager.cancel(0);
    }

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
//      Log.d("seqlog", "Main.onCreate()");

        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        mIntent = new Intent(this, Service.class);
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);

        // サービス開始／停止ボタン
        final App app = ((App)getApplication());

        mStartStopService = (Button)findViewById(R.id.startStopService);
        mStartStopService.setText(
            getString(app.isRunning() == false
                ? R.string.start_service
                : R.string.stop_service));

        mStartStopService.setOnClickListener(new OnClickListener()
        {
            public void onClick(View v)
            {
                if (app.isRunning() == false)
                {
                    start();
                }
                else
                {
                    if (app.canStop())
                        stop();
                }
            }
        });

        // 共有メモリ設定エリア
        String pathName = sp.getString("sharedMemoryPathName", getString(R.string.default_shared_memory_path));

        mSharedMemoryPathName = (EditText)findViewById(R.id.sharedMemoryPathName);
        mSharedMemoryPathName.setText(pathName);

        // ログ出力ディレクトリ
        String logOutputDir = sp.getString("logOutputDir", getString(R.string.default_log_output_dir));

        mLogOutputDir = (EditText)findViewById(R.id.logOutputDir);
        mLogOutputDir.setText(logOutputDir);

        // 最大ログファイルサイズ 
        Integer maxFileSize = sp.getInt("maxFileSize", 0);

        mMaxFileSize = (EditText)findViewById(R.id.maxFileSize);
        mMaxFileSize.setText(maxFileSize.toString());

        String maxFileSizeUnit = sp.getString("maxFileSizeUnit", "KB");
        mMaxFileSizeUnit = (Spinner)findViewById(R.id.maxFileSizeUnit);

        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        adapter.add("KB");
        adapter.add("MB");
        
        mMaxFileSizeUnit.setAdapter(adapter);
        mMaxFileSizeUnit.setSelection(maxFileSizeUnit.equals("KB") ? 0 : 1);

        // 最大ログファイル数
        Integer maxFileCount = sp.getInt("maxFileCount", 0);

        mMaxFileCount = (EditText)findViewById(R.id.maxFileCount);
        mMaxFileCount.setText(maxFileCount.toString());

        // Web Server ポート
        Integer webServerPort = sp.getInt("webServerPort", 8080);

        mWebServerPort = (EditText)findViewById(R.id.webServerPort);
        mWebServerPort.setText(webServerPort.toString());

        // Sequence Log Server IP
        String sequenceLogServerIp = sp.getString("sequenceLogServerIp", "192.168.0.2");

        mSequenceLogServerIp = (EditText)findViewById(R.id.sequenceLogServerIp);
        mSequenceLogServerIp.setText(sequenceLogServerIp);

        // Sequence Log Server ポート
        Integer sequenceLogServerPort = sp.getInt("sequenceLogServerPort", 8081);

        mSequenceLogServerPort = (EditText)findViewById(R.id.sequenceLogServerPort);
        mSequenceLogServerPort.setText(sequenceLogServerPort.toString());
    }

    @Override
    protected void onDestroy()
    {
//      Log.d("seqlog", "Main.onDestroy()");
        super.onDestroy();

        App app = (App)getApplication();
        setSetting();

        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);
        SharedPreferences.Editor editor = sp.edit();

        editor.putString("sharedMemoryPathName",  app.mSharedMemoryPathName);
        editor.putString("logOutputDir",          app.mLogOutputDir);
        editor.putInt(   "maxFileSize",           app.mMaxFileSize);
        editor.putString("maxFileSizeUnit",       app.mMaxFileSizeUnit);
        editor.putInt(   "maxFileCount",          app.mMaxFileCount);
        editor.putInt(   "webServerPort",         app.mWebServerPort);
        editor.putString("sequenceLogServerIp",   app.mSequenceLogServerIp);
        editor.putInt(   "sequenceLogServerPort", app.mSequenceLogServerPort);

        editor.commit();
    }
}
