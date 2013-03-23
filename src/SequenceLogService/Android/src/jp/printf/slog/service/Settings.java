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

import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.annotation.SuppressLint;
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
    private final String        KEY_START_STOP =               "startStop";
    private final String        KEY_SHARED_MEMORY_PATH_NAME =  "sharedMemoryPathName";
    private final String        KEY_LOG_OUTPUT_DIR =           "logOutputDir";
    private final String        KEY_MAX_FILE_SIZE =            "maxFileSize";
    private final String        KEY_MAX_FILE_SIZE_UNIT =       "maxFileSizeUnit";
    private final String        KEY_MAX_FILE_COUNT =           "maxFileCount";
    private final String        KEY_CHANGE_SUPER_USER =        "superUser";
    private final String        KEY_WEB_SERVER_PORT =          "webServerPort";
    private final String        KEY_SEQUENCE_LOG_SERVER_IP =   "sequenceLogServerIp";
    private final String        KEY_SEQUENCE_LOG_SERVER_PORT = "sequenceLogServerPort";

    private Intent              mServiceIntent;         // Sequence Log Service を開始 / 停止するためのインテント
    private SharedPreferences   mSP;

    private int                 mWebServerPort;         // Web Server ポート
    private String              mSequenceLogServerIp;   // Sequence Log Server IP
    private int                 mSequenceLogServerPort; // Sequence Log Server ポート

    /**
     * すべてのサマリーを更新する
     * 
     * Appクラスの各メンバの更新も行う
     */
    @SuppressLint("DefaultLocale")
    private void updateSummaries()
    {
        App app = (App)getActivity().getApplication();
        String value;

        // Sequence Log Service 開始 / 停止
        boolean isRunning = mSP.getBoolean(KEY_START_STOP, false);

        findPreference(KEY_START_STOP).setSummary(
            isRunning
                ? getString(R.string.running)
                : getString(R.string.stopping));

        // 共有メモリパス
        value = updateSummary(KEY_SHARED_MEMORY_PATH_NAME, (isRunning == false));
        app.mSharedMemoryPathName = value;

        // ログ出力ディレクトリ
        value = updateSummary(KEY_LOG_OUTPUT_DIR, (isRunning == false));
        app.mLogOutputDir = value;

        // 最大ファイルサイズ
        value = updateSummary(KEY_MAX_FILE_SIZE, (isRunning == false));
        app.mMaxFileSize = Integer.parseInt(value);

        // 最大ファイルサイズ単位
        value = updateSummary(KEY_MAX_FILE_SIZE_UNIT, (isRunning == false));
        app.mMaxFileSizeUnit = value;

        // 最大ファイル数
        value = updateSummary(KEY_MAX_FILE_COUNT, (isRunning == false));
        app.mMaxFileCount = Integer.parseInt(value);

        // スーパーユーザー
        Preference pref = findPreference(KEY_CHANGE_SUPER_USER);
        pref.setEnabled(isRunning == false);

        // Sequence Log Service Web Server ポート
        value = updateSummary(KEY_WEB_SERVER_PORT, true);
        mWebServerPort = Integer.parseInt(value);

        // Sequence Log Server IP
        value = updateSummary(KEY_SEQUENCE_LOG_SERVER_IP, true);
        mSequenceLogServerIp = value;

        // Sequence Log Server ポート
        value = updateSummary(KEY_SEQUENCE_LOG_SERVER_PORT, true);
        mSequenceLogServerPort = Integer.parseInt(value);

        // Sequence Log Service Web
        LinkPreference linkPref = (LinkPreference)findPreference("serviceWeb");
        String url = String.format("http://localhost:%d", mWebServerPort);

        linkPref.setSummary(url);
        linkPref.setUrl(url);

        // 設定反映
        app.updateSettings();
        app.setSequenceLogServer(mSequenceLogServerIp, mSequenceLogServerPort);
    }

    /**
     * サマリーを更新する
     * 
     * @param   key     SharedPreferencesのキー
     * @param   enabled 有効/無効化フラグ
     * 
     * @return  keyで指定されたSharedPreferencesの値
     */
    private String updateSummary(String key, boolean enabled)
    {
        Preference pref = findPreference(key);
        String value = mSP.getString(key, "");

        pref.setSummary(value);
        pref.setEnabled(enabled);
        return value;
    }

    /**
     * Sequence Log Service を開始する
     */
    private void start()
    {
        Activity activity = getActivity();
        activity.startService(mServiceIntent);

        // Notificationタッチ時のインテント
        Intent activityIntent = new Intent(activity, Main.class);
        activityIntent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_NEW_TASK);

        // Notificationに設定するインテント
        PendingIntent contentIntent = PendingIntent.getActivity(activity, 0, activityIntent, Intent.FLAG_ACTIVITY_NEW_TASK);

        // Notification生成
        Notification notification = new Notification.Builder(activity)
            .setSmallIcon(R.drawable.icon)
            .setTicker(getString(R.string.start_service))
            .setWhen(System.currentTimeMillis())
            .setContentTitle(getText(R.string.app_name))
            .setContentText("Started.")
            .setContentIntent(contentIntent)
            .build();

        // NotificationManagerにセット
        NotificationManager manager = (NotificationManager)activity.getSystemService(Activity.NOTIFICATION_SERVICE);
        manager.notify(0, notification);
    }

    /**
     * Sequence Log Service を停止する
     */
    private void stop()
    {
        Activity activity = getActivity();
        activity.stopService(mServiceIntent);

        NotificationManager manager = (NotificationManager)activity.getSystemService(Activity.NOTIFICATION_SERVICE);
        manager.cancel(0);
    }

    /**
     * onCreate
     */
    public void onCreate(Bundle savedInstanceState)
    {
//      Log.d("seqlog", "Main.onCreate()");

        super.onCreate(savedInstanceState);

        // Preference追加
        addPreferencesFromResource(R.xml.settings);

        // メンバ初期化
        Activity activity = getActivity();

        mServiceIntent = new Intent(activity, Service.class);
        mSP = getPreferenceScreen().getSharedPreferences();

        // サマリー更新
        updateSummaries();

        // Sequence Log Service Web Server ポート設定
        App app = (App)activity.getApplication();
        app.setWebServerPort(mWebServerPort);

        // スーパーユーザー
        setSuperUser();

        // Sequence Log Service の開始
        if (app.isRunning() == false && mSP.getBoolean(KEY_START_STOP, false))
            start();
    }

    /**
     * onResume
     */
    public void onResume()
    {
        super.onResume();
        mSP.registerOnSharedPreferenceChangeListener(this);
    }

    /**
     * onPause
     */
    public void onPause()
    {
        super.onPause();
        mSP.unregisterOnSharedPreferenceChangeListener(this);
    }

    /**
     * onDestroy
     */
    public void onDestroy()
    {
//      Log.d("seqlog", "Main.onDestroy()");
        super.onDestroy();
    }

    /**
     * onSharedPreferenceChanged
     */
    public void onSharedPreferenceChanged(SharedPreferences sharedPreferences,  String key)
    {
        App app = (App)getActivity().getApplication();
        updateSummaries();

        // Sequence Log Service の開始 / 停止
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

        // スーパーユーザー
        if (key.equals(KEY_CHANGE_SUPER_USER))
        {
            setSuperUser();
        }

        // Sequence Log Service Web Server ポート
        if (key.equals(KEY_WEB_SERVER_PORT))
        {
            app.setWebServerPort(mWebServerPort);
        }
    }

    /**
     * スーパーユーザー設定
     */
    private void setSuperUser()
    {
        App app = (App)getActivity().getApplication();
        boolean on = mSP.getBoolean(KEY_CHANGE_SUPER_USER, false);

        if (app.setSuperUser(on) == false)
        {
            CheckBoxPreference pref = (CheckBoxPreference)findPreference(KEY_CHANGE_SUPER_USER);
            pref.setChecked(false);
        }
    }
}
