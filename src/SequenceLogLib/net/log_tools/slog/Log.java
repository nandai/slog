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
package net.log_tools.slog;

public final class Log
{
    // 出力フラグ
    public  static final int KEEP =         0;      // シーケンスログの出力をキープする
    public  static final int OUTPUT_ALL =   1;      // キープ中のシーケンスログも含め出力する
    public  static final int ALWAYS =       2;      // キープ中のシーケンスログを出力し、さらに配下のシーケンスログは即座に出力する
    public  static final int ROOT =         3;      // シーケンスログサービスでルートをオンにするとALWAYSとして、オフにするとKEEPとして扱う

    // ログレベル
    private static final int DEBUG =        0;      // デバッグ
    private static final int INFO =         1;      // 情報
    private static final int WARN =         2;      // 警告
    private static final int ERROR =        3;      // エラー

    /**
     * シーケンスログファイル名設定
     */
    public native static void setFileName(String name);
    public native static void setRootFlag(int outputFlag);

    /**
     * ステップイン
     */
    public         static long stepIn(String className, String funcName) {return stepIn(className, funcName, KEEP);}
    public  native static long stepIn(String className, String funcName, int outputFlag);

    public         static long stepIn(int classID,      String funcName) {return stepIn(classID,   funcName, KEEP);}
    public  native static long stepIn(int classID,      String funcName, int outputFlag);

    public         static long stepIn(int classID,      int    funcID)   {return stepIn(classID,   funcID,   KEEP);}
    public  native static long stepIn(int classID,      int    funcID,   int outputFlag);

    /**
     * ステップアウト
     */
    public native static void stepOut(long slog);

    /**
     * メッセージ出力
     */
    private native static void message(int level, String message,   long slog/*tag*/);
    private native static void message(int level, int    messageID, long slog/*tag*/);

    private native static void message(int level, String message,   String tag);

    public static void v(long slog/*tag*/, String message)              {message(DEBUG, message,   slog);}
    public static void d(long slog/*tag*/, String message)              {message(DEBUG, message,   slog);}
    public static void i(long slog/*tag*/, String message)              {message(INFO,  message,   slog);}
    public static void w(long slog/*tag*/, String message)              {message(WARN,  message,   slog);}
    public static void e(long slog/*tag*/, String message)              {message(ERROR, message,   slog);}

    public static void v(long slog/*tag*/, int messageID)               {message(DEBUG, messageID, slog);}
    public static void d(long slog/*tag*/, int messageID)               {message(DEBUG, messageID, slog);}
    public static void i(long slog/*tag*/, int messageID)               {message(INFO,  messageID, slog);}
    public static void w(long slog/*tag*/, int messageID)               {message(WARN,  messageID, slog);}
    public static void e(long slog/*tag*/, int messageID)               {message(ERROR, messageID, slog);}

    // Androidに適用する時にandroid.util.Log()として修正しなくても良いようにするためのメソッド。
    // Throwableは無視する。
    public static void w(long slog/*tag*/, String message, Throwable e) {message(WARN,  message,   slog);}
    public static void e(long slog/*tag*/, String message, Throwable e) {message(ERROR, message,   slog);}

    public static void v(String tag,       String message)              {message(DEBUG, message,   tag);}
    public static void d(String tag,       String message)              {message(DEBUG, message,   tag);}
    public static void i(String tag,       String message)              {message(INFO,  message,   tag);}
    public static void w(String tag,       String message)              {message(WARN,  message,   tag);}
    public static void e(String tag,       String message)              {message(ERROR, message,   tag);}

    public static void w(String tag,       String message, Throwable e) {message(WARN,  message,   tag);}
    public static void e(String tag,       String message, Throwable e) {message(ERROR, message,   tag);}
}
