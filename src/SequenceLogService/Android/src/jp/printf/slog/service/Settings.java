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

import jp.printf.slog.Log;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.SwitchPreference;
import android.text.SpannableString;
import android.text.style.ForegroundColorSpan;
import android.view.Gravity;
import android.widget.Toast;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.graphics.Color;
import android.os.Bundle;
//port android.util.Log;

public class Settings extends PreferenceFragment implements OnSharedPreferenceChangeListener
{
    private final String CLS_NAME = "Settings";

    private final String        KEY_START_STOP =               "startStop";
    private final String        KEY_LOG_OUTPUT_DIR =           "logOutputDir";
    private final String        KEY_MAX_FILE_SIZE =            "maxFileSize";
    private final String        KEY_MAX_FILE_SIZE_UNIT =       "maxFileSizeUnit";
    private final String        KEY_MAX_FILE_COUNT =           "maxFileCount";
    private final String        KEY_CHANGE_SUPER_USER =        "superUser";
    private final String        KEY_WEB_SERVER_PORT =          "webServerPort";
//  private final String        KEY_WEB_SERVER_PORT_SSL =      "webServerPortSSL";
    private final String        KEY_SEQUENCE_LOG_SERVER_PORT = "sequenceLogServerPort";

    private boolean             mStarting = false;
    private Intent              mServiceIntent;         // Sequence Log Service を開始 / 停止するためのインテント
    private SharedPreferences   mSP;

    private boolean             mLogOutputDirOkFlag = true;

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
        Preference pref = findPreference(  KEY_START_STOP);
        boolean isRunning = mSP.getBoolean(KEY_START_STOP, false);

        setTitleColor(pref);
        pref.setSummary(isRunning
            ? getString(R.string.running)
            : getString(R.string.stopping));

        // ログ出力ディレクトリ
        value = updateSummary(KEY_LOG_OUTPUT_DIR, (isRunning == false));
        app.mLogOutputDir = value;

        if (mLogOutputDirOkFlag == false)
        {
            pref = findPreference(KEY_LOG_OUTPUT_DIR);
            setErrorSummaryColor(pref);
        }

        // 最大ファイルサイズ
        value = updateSummary(KEY_MAX_FILE_SIZE, (isRunning == false));
        app.mMaxFileSize = parseInt(value);

        // 最大ファイルサイズ単位
        value = updateSummary(KEY_MAX_FILE_SIZE_UNIT, (isRunning == false));
        app.mMaxFileSizeUnit = value;

        // 最大ファイル数
        value = updateSummary(KEY_MAX_FILE_COUNT, (isRunning == false));
        app.mMaxFileCount = parseInt(value);

        // スーパーユーザー
        pref = findPreference(KEY_CHANGE_SUPER_USER);
        boolean isSuperUser = mSP.getBoolean(KEY_CHANGE_SUPER_USER, false);

        setTitleColor(pref);
        pref.setSummary(isSuperUser
            ? getString(R.string.super_user)
            : getString(R.string.general_user));
        pref.setEnabled(isRunning == false);

        // Sequence Log Service Web Server ポート
        value = updateSummary(KEY_WEB_SERVER_PORT, (isRunning == false));
        app.mWebServerPort = parseInt(value);

        // Sequence Log Service Web Server ポート (SSL)
//      value = updateSummary(KEY_WEB_SERVER_PORT_SSL, (isRunning == false));
//      app.mWebServerPortSSL = Integer.parseInt(value);

        // Sequence Log Server ポート
        value = updateSummary(KEY_SEQUENCE_LOG_SERVER_PORT, (isRunning == false));
        app.mSequenceLogServerPort = parseInt(value);

        // Sequence Log Service Web
        LinkPreference linkPref = (LinkPreference)findPreference("serviceWeb");
        String url = String.format("http://localhost:%d", app.mWebServerPort);

        setTitleColor(linkPref);
        linkPref.setSummary(url);
        linkPref.setUrl(url);

        // official home page
        pref = findPreference("officialHomePage");
        setTitleColor(pref);

        // gitHub
        pref = findPreference("gitHub");
        setTitleColor(pref);
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

        setTitleColor(pref);
        pref.setSummary(value);
        pref.setEnabled(enabled);
        return value;
    }

    /**
     * 文字列を数値に変換
     * 
     * @param value 変換する文字列
     * @return      変換結果
     */
    private int parseInt(String value)
    {
        try
        {
            return Integer.parseInt(value);
        }
        catch (NumberFormatException e)
        {
            return 0;
        }
    }

    /**
     * Preferenceのタイトルカラーを設定する
     */
    private void setTitleColor(Preference pref)
    {
        SpannableString title = new SpannableString(pref.getTitle());
        title.setSpan(new ForegroundColorSpan(Color.rgb(144, 255, 144)), 0, title.length(), 0);

        pref.setTitle(title);
    }

    /**
     * Preferenceのサマリーカラーを設定する
     */
    private void setErrorSummaryColor(Preference pref)
    {
        SpannableString summary = new SpannableString(pref.getSummary());
        summary.setSpan(new ForegroundColorSpan(Color.RED), 0, summary.length(), 0);

        pref.setSummary(summary);
    }

    /**
     * Sequence Log Service を開始する
     */
    private void start()
    {
        mStarting = true;

        App app = (App)getActivity().getApplication();
        int result = app.start();
        boolean success = true;

        if ((result & 0x02) != 0)
        {
            success = false;
            mLogOutputDirOkFlag = false;
        }

        if (success == false)
        {
            SwitchPreference pref = (SwitchPreference)findPreference(KEY_START_STOP);
            pref.setChecked(false);
            updateSummaries();

            Toast toast = Toast.makeText(getActivity(), getString(R.string.permission_denied), Toast.LENGTH_SHORT);
            toast.setGravity(Gravity.CENTER, 0, 0);
            toast.show();

            mStarting = false;
            return;
        }

        mLogOutputDirOkFlag = true;
        updateSummaries();

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

        mStarting = false;
    }

    /**
     * Sequence Log Service を停止する
     */
    private void stop()
    {
        App app = (App)getActivity().getApplication();
        app.stop();

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
        long TAG = Log.stepIn(CLS_NAME, "onCreate");
        super.onCreate(savedInstanceState);

        // Preference追加
        addPreferencesFromResource(R.xml.settings);

        // メンバ初期化
        Activity activity = getActivity();

        mServiceIntent = new Intent(activity, Service.class);
        mSP = getPreferenceScreen().getSharedPreferences();

        // サマリー更新
        updateSummaries();

        // スーパーユーザー
        setSuperUser();

        // Sequence Log Service の開始
        App app = (App)activity.getApplication();

        if (app.getServiceStatus() == app.SERVICE_STOPPED && mSP.getBoolean(KEY_START_STOP, false))
            start();

        Log.stepOut(TAG);
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
        super.onDestroy();
    }

    /**
     * onSharedPreferenceChanged
     */
    public void onSharedPreferenceChanged(SharedPreferences sharedPreferences,  String key)
    {
        App app = (App)getActivity().getApplication();

        if (key.equals(KEY_LOG_OUTPUT_DIR))
            mLogOutputDirOkFlag = true;

        updateSummaries();

        // Sequence Log Service の開始 / 停止
        if (key.equals(KEY_START_STOP))
        {
            if (mStarting)
                return;

            if (app.getServiceStatus() == app.SERVICE_STOPPED)
            {
                start();
            }
            else
            {
                stop();
            }
        }

        // スーパーユーザー
        if (key.equals(KEY_CHANGE_SUPER_USER))
        {
            setSuperUser();
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
