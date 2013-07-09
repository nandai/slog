#!/bin/sh

javac -verbose -encoding utf8 jp/printf/slog/Log.java
javac -verbose -encoding utf8 jp/printf/WebSocketClient.java
javac -verbose -encoding utf8 jp/printf/WebSocketListener.java

jar cvf slog.jar              jp/printf/slog/Log.class \
                              jp/printf/WebSocketClient.class \
                              jp/printf/WebSocketListener.class

javah -verbose -classpath .   jp.printf.slog.Log
javah -verbose -classpath .   jp.printf.WebSocketClient

rm jp/printf/slog/Log.class \
   jp/printf/WebSocketClient.class \
   jp/printf/WebSocketListener.class

mv jp_printf_slog_Log.h \
   jp_printf_WebSocketClient.h \
   src/
mv slog.jar ../../bin/Java/
