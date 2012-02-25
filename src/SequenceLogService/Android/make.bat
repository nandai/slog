@echo off
cd %~dp0

call :exec javah -verbose -classpath bin/classes net.log_tools.slog.service.App

echo.
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
