#!/bin/sh

javac -verbose -encoding utf8 jp/printf/slog/Log.java
jar cvf slog.jar              jp/printf/slog/Log.class
javah -verbose -classpath .   jp.printf.slog.Log

rm jp/printf/slog/Log.class
mv jp_printf_slog_Log.h src/
mv slog.jar ../../bin/Java/
