@echo off
cd %~dp0

call :exec javac -verbose -encoding utf8 net/log_tools/slog/Log.java
call :exec jar cvf slog.jar              net/log_tools/slog/Log.class
call :exec javah -verbose -classpath .   net.log_tools.slog.Log

echo.
del  net\log_tools\slog\Log.class
move net_log_tools_slog_Log.h src/
move               slog.jar ../../bin/Java/

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
