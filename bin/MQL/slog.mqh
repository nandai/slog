//+------------------------------------------------------------------+
//|                                                         slog.mqh |
//|                                    Copyright (c) 2014, printf.jp |
//|                                             http://www.printf.jp |
//+------------------------------------------------------------------+
#property copyright "Copyright (c) 2014, printf.jp"
#property link      "http://www.printf.jp"

#import "slog.dll"
    void slog_loadConfig(string fileName);
    int  slog_stepIn(string className, string funcName);
    void slog_stepOut(int tag);
    void slog_message(int tag, int level, string message);
    void slog_assert( int tag, string assertName, bool result);
#import

#define SLOG_DEBUG 0
#define SLOG_INFO  1
#define SLOG_WARN  2
#define SLOG_ERROR 3
