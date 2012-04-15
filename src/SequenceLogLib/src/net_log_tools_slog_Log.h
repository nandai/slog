/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class net_log_tools_slog_Log */

#ifndef _Included_net_log_tools_slog_Log
#define _Included_net_log_tools_slog_Log
#ifdef __cplusplus
extern "C" {
#endif
#undef net_log_tools_slog_Log_KEEP
#define net_log_tools_slog_Log_KEEP 0L
#undef net_log_tools_slog_Log_OUTPUT_ALL
#define net_log_tools_slog_Log_OUTPUT_ALL 1L
#undef net_log_tools_slog_Log_ALWAYS
#define net_log_tools_slog_Log_ALWAYS 2L
#undef net_log_tools_slog_Log_ROOT
#define net_log_tools_slog_Log_ROOT 3L
#undef net_log_tools_slog_Log_DEBUG
#define net_log_tools_slog_Log_DEBUG 0L
#undef net_log_tools_slog_Log_INFO
#define net_log_tools_slog_Log_INFO 1L
#undef net_log_tools_slog_Log_WARN
#define net_log_tools_slog_Log_WARN 2L
#undef net_log_tools_slog_Log_ERROR
#define net_log_tools_slog_Log_ERROR 3L
/*
 * Class:     net_log_tools_slog_Log
 * Method:    setFileName
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_log_1tools_slog_Log_setFileName
  (JNIEnv *, jclass, jstring);

/*
 * Class:     net_log_tools_slog_Log
 * Method:    setRootFlag
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_net_log_1tools_slog_Log_setRootFlag
  (JNIEnv *, jclass, jint);

/*
 * Class:     net_log_tools_slog_Log
 * Method:    stepIn
 * Signature: (Ljava/lang/String;Ljava/lang/String;I)J
 */
JNIEXPORT jlong JNICALL Java_net_log_1tools_slog_Log_stepIn__Ljava_lang_String_2Ljava_lang_String_2I
  (JNIEnv *, jclass, jstring, jstring, jint);

/*
 * Class:     net_log_tools_slog_Log
 * Method:    stepIn
 * Signature: (ILjava/lang/String;I)J
 */
JNIEXPORT jlong JNICALL Java_net_log_1tools_slog_Log_stepIn__ILjava_lang_String_2I
  (JNIEnv *, jclass, jint, jstring, jint);

/*
 * Class:     net_log_tools_slog_Log
 * Method:    stepIn
 * Signature: (III)J
 */
JNIEXPORT jlong JNICALL Java_net_log_1tools_slog_Log_stepIn__III
  (JNIEnv *, jclass, jint, jint, jint);

/*
 * Class:     net_log_tools_slog_Log
 * Method:    stepOut
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_net_log_1tools_slog_Log_stepOut
  (JNIEnv *, jclass, jlong);

/*
 * Class:     net_log_tools_slog_Log
 * Method:    message
 * Signature: (ILjava/lang/String;J)V
 */
JNIEXPORT void JNICALL Java_net_log_1tools_slog_Log_message__ILjava_lang_String_2J
  (JNIEnv *, jclass, jint, jstring, jlong);

/*
 * Class:     net_log_tools_slog_Log
 * Method:    message
 * Signature: (IIJ)V
 */
JNIEXPORT void JNICALL Java_net_log_1tools_slog_Log_message__IIJ
  (JNIEnv *, jclass, jint, jint, jlong);

/*
 * Class:     net_log_tools_slog_Log
 * Method:    message
 * Signature: (ILjava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_net_log_1tools_slog_Log_message__ILjava_lang_String_2Ljava_lang_String_2
  (JNIEnv *, jclass, jint, jstring, jstring);

#ifdef __cplusplus
}
#endif
#endif
