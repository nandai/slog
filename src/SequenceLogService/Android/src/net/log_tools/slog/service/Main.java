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
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.Spinner;
//port android.util.Log;

public class Main extends Activity
{
	private static final String	START_TEXT =      "Start Sequence Log Service";
	private static final String	STOP_TEXT =       "Stop Sequence Log Service";

	private static final String	CONNECT_TEXT =    "Connect sequence log print";
	private static final String	DISCONNECT_TEXT	= "Disconnect sequence log print";

	private Intent		mIntent;
	
	private Button		mStartStopService;			// サービス開始／終了ボタン
	private EditText	mSharedMemoryPathName;		// 共有メモリ設定エリア
	private Button		mConnDisconnPrint;			// シーケンスログプリント接続／切断ボタン
	private EditText	mIPAddress;					// シーケンスログプリントIPアドレス設定エリア
	private EditText	mLogOutputDir;				// ログ出力ディレクトリ
	private EditText	mMaxFileSize;				// 最大ログファイルサイズ
	private Spinner		mMaxFileSizeUnit;			// 最大ログファイルサイズ単位
	private EditText	mMaxFileCount;				// 最大ログファイル数
	private CheckBox	mRootAlways;				// ROOTをALWAYSとするかどうか

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

		app.mRootAlways = mRootAlways.isChecked();
	}

	private void start()
	{
		setSetting();

    	mStartStopService.setText(STOP_TEXT);
    	startService(mIntent);

    	PendingIntent contentIntent = PendingIntent.getActivity(this, 0, new Intent(this, Main.class), Intent.FLAG_ACTIVITY_NEW_TASK);

    	Notification notification = new Notification(R.drawable.icon, START_TEXT, System.currentTimeMillis());
    	notification.setLatestEventInfo(this, getText(R.string.app_name), "Started.", contentIntent);

    	NotificationManager manager = (NotificationManager)getSystemService(NOTIFICATION_SERVICE);
    	manager.notify(0, notification);
	}

	private void stop()
	{
    	mStartStopService.setText(START_TEXT);
    	stopService(mIntent);

    	NotificationManager manager = (NotificationManager)getSystemService(NOTIFICATION_SERVICE);
    	manager.cancel(0);
	}

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
//		Log.d("seqlog", "Main.onCreate()");

		super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

		mIntent = new Intent(this, Service.class);
		SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);

		// サービス開始／停止ボタン
		final App app = ((App)getApplication());

		mStartStopService = (Button)findViewById(R.id.startStopService);
		mStartStopService.setText(app.isRunning() == false ? START_TEXT : STOP_TEXT);

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
		String pathName = sp.getString("sharedMemoryPathName", "/sdcard/slog");

		mSharedMemoryPathName = (EditText)findViewById(R.id.sharedMemoryPathName);
		mSharedMemoryPathName.setText(pathName);
  
        // シーケンスログプリント接続／切断ボタン
		mConnDisconnPrint = (Button)findViewById(R.id.connDisconnPrint);
		mConnDisconnPrint.setText(app.isConnecting() == false ? CONNECT_TEXT : DISCONNECT_TEXT);

		mConnDisconnPrint.setOnClickListener(new OnClickListener()
		{
			public void onClick(View v)
			{
        		if (app.isConnecting() == false)
        		{
        			SpannableStringBuilder sb = (SpannableStringBuilder)mIPAddress.getText();
        			String ipAddress = sb.toString();

        			if (app.connectSequenceLogPrint(ipAddress) == true)
        			{
        				app.connecting(true);
        				mConnDisconnPrint.setText(DISCONNECT_TEXT);
        			}
        		}
        		else
        		{
        			app.disconnectSequenceLogPrint();
        			app.connecting(false);
    				mConnDisconnPrint.setText(CONNECT_TEXT);
        		}
			}
		});
		
        // シーケンスログプリントIPアドレス設定
		String ipAddress = sp.getString("ipAddress", "127.0.0.1");

		mIPAddress = (EditText)findViewById(R.id.ipAddress);
		mIPAddress.setText(ipAddress);

		// ログ出力ディレクトリ
		String logOutputDir = sp.getString("logOutputDir", "/sdcard/slog/log");

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
		
		// ROOTをALWAYSとするかどうか
		boolean rootAlways = sp.getBoolean("rootAlways", true);
		
		mRootAlways = (CheckBox)findViewById(R.id.rootAlways);
		mRootAlways.setChecked(rootAlways);
    }

	@Override
	protected void onDestroy()
	{
//		Log.d("seqlog", "Main.onDestroy()");
		super.onDestroy();

		App app = (App)getApplication();
		setSetting();

		SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);
		SharedPreferences.Editor editor = sp.edit();

		SpannableStringBuilder sb;
		sb = (SpannableStringBuilder)mIPAddress.getText();
		String ipAddress = sb.toString();

        editor.putString("sharedMemoryPathName", app.mSharedMemoryPathName);
		editor.putString("ipAddress",            ipAddress);
		editor.putString("logOutputDir",         app.mLogOutputDir);
		editor.putInt(   "maxFileSize",          app.mMaxFileSize);
		editor.putString("maxFileSizeUnit",      app.mMaxFileSizeUnit);
		editor.putInt(   "maxFileCount",         app.mMaxFileCount);
		editor.putBoolean("rootAlways",          app.mRootAlways);

		editor.commit();
	}
}
