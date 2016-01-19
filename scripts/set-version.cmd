@echo off
IF /I "%NSBUILDENV%"=="" GOTO :BADENV

git log -1 --format=%%h > tmp.txt
set /P VAR=<tmp.txt
%NSBUILDENV%\bin\date.exe +%%Y%%m%%d-r%VAR% > SVN_VERSION_FILE
del tmp.txt
goto :EOF

:BADENV
echo Bad environment. Add NSBUILDENV environment variable with paths to:
echo     Qt 4.7.4, MinGW 4.4.0, GnuWin32: date, bison, flex, libiconv,
echo     regex, libintl
echo.

:EOF