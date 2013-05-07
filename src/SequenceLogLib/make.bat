@echo off
cd %~dp0

call :exec javac -verbose -encoding utf8 jp/printf/slog/Log.java
call :exec jar cvf slog.jar              jp/printf/slog/Log.class
call :exec javah -verbose -classpath .   jp.printf.slog.Log

echo.
del  jp\printf\slog\Log.class
move jp_printf_slog_Log.h src/
move slog.jar ../../bin/Java/

exit /b 0

rem ***************************************************************************
rem * sub routine
rem ***************************************************************************
:exec
echo.
echo --------------------------------------------------------------------------
echo ^> %*
echo.
%*
exit /b 0
