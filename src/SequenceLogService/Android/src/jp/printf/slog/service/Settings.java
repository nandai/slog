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

import android.preference.PreferenceFragment;
import android.app.Activity;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.os.Bundle;
//port android.util.Log;

public class Settings extends PreferenceFragment implements OnSharedPreferenceChangeListener
{
    private Intent      mIntent;

    private void updateSummaries()
    {
        App app = (App)getActivity().getApplication();
        SharedPreferences sp = getPreferenceScreen().getSharedPreferences();
        String key;
        String value;

        // Sequence Log Service 開始 / 停止
        key = "startStop";
        findPreference(key).setSummary(
            sp.getBoolean(key, false)
                ? getString(R.string.running)
                : getString(R.string.stopping));

        // 共有メモリパス
        value = setSummary(sp, "sharedMemoryPathName");
        app.mSharedMemoryPathName = value;

        // ログ出力ディレクトリ
        value = setSummary(sp, "logOutputDir");
        app.mLogOutputDir = value;

        // 最大ファイルサイズ
        value = setSummary(sp, "maxFileSize");
        app.mMaxFileSize = Integer.parseInt(value);

        // 最大ファイルサイズ単位
        value = setSummary(sp, "maxFileSizeUnit");
        app.mMaxFileSizeUnit = value;

        // 最大ファイル数
        value = setSummary(sp, "maxFileCount");
        app.mMaxFileCount = Integer.parseInt(value);

        // Sequence Log Service Web Server ポート
        value = setSummary(sp, "webServerPort");
        app.mWebServerPort = Integer.parseInt(value);

        // Sequence Log Server IP
        value = setSummary(sp, "sequenceLogServerIp");
        app.mSequenceLogServerIp = value;

        // Sequence Log Server ポート
        value = setSummary(sp, "sequenceLogServerPort");
        app.mSequenceLogServerPort = Integer.parseInt(value);
    }

    private String setSummary(SharedPreferences sp, String key)
    {
        String value = sp.getString(key, "");

        findPreference(key).setSummary(value);
        return value;
    }

    private void start()
    {
        Activity activity = getActivity();
        activity.startService(mIntent);

        Intent activityIntent = new Intent(activity, Main.class);
        activityIntent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_NEW_TASK);

        PendingIntent contentIntent = PendingIntent.getActivity(activity, 0, activityIntent, Intent.FLAG_ACTIVITY_NEW_TASK);

        Notification notification = new Notification.Builder(activity)
            .setSmallIcon(R.drawable.icon)
            .setTicker(getString(R.string.start_service))
            .setWhen(System.currentTimeMillis())
            .setContentTitle(getText(R.string.app_name))
            .setContentText("Started.")
            .setContentIntent(contentIntent)
            .build();

        NotificationManager manager = (NotificationManager)activity.getSystemService(Activity.NOTIFICATION_SERVICE);
        manager.notify(0, notification);
    }

    private void stop()
    {
        Activity activity = getActivity();
        activity.stopService(mIntent);

        NotificationManager manager = (NotificationManager)activity.getSystemService(Activity.NOTIFICATION_SERVICE);
        manager.cancel(0);
    }

    /** Called when the activity is first created. */
    public void onCreate(Bundle savedInstanceState)
    {
//      Log.d("seqlog", "Main.onCreate()");

        super.onCreate(savedInstanceState);

        Activity activity = getActivity();
        mIntent = new Intent(activity, Service.class);

        addPreferencesFromResource(R.xml.settings);
        updateSummaries();

        App app = (App)activity.getApplication();

        if (app.isRunning() == false && getPreferenceScreen().getSharedPreferences().getBoolean("startStop", false))
            start();
    }

    public void onResume()
    {
        super.onResume();
        getPreferenceScreen().getSharedPreferences().registerOnSharedPreferenceChangeListener(this);
    }

    public void onPause()
    {
        super.onPause();
        getPreferenceScreen().getSharedPreferences().unregisterOnSharedPreferenceChangeListener(this);
    }

    public void onDestroy()
    {
//      Log.d("seqlog", "Main.onDestroy()");
        super.onDestroy();
    }

    public void onSharedPreferenceChanged(SharedPreferences sharedPreferences,  String key)
    {
        App app = (App)getActivity().getApplication();
        updateSummaries();

        if (key.equals("startStop"))
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
    }
}
