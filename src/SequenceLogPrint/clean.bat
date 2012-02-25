@echo off
cd %~dp0

call :getTargetName %~p0.
rmdir /s /q ..\obj\%TARGET%
del %TARGET%.ncb
del %TARGET%.suo /ah
del %TARGET%.vcproj.*.user
del src\*.aps
exit /b 0

:getTargetName
set TARGET=%~nx1
exit /b 0
