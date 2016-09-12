@echo off
IF /I "%NSBUILDENV%"=="" GOTO :BADENV

SET /P VERSION=<SVN_VERSION_FILE
md nsbuilder1-%VERSION%-win\
copy release\ nsbuilder1-%VERSION%-win\
7z a pack/nsbuilder1-%VERSION%-win.zip nsbuilder1-%VERSION%-win/*
del /s /q nsbuilder1-%VERSION%-win
rmdir nsbuilder1-%VERSION%-win

goto :EOF

:BADENV
echo Bad environment. Add NSBUILDENV environment variable with paths to:
echo     GnuWin32: date, bison, flex, libiconv, regex, libintl
echo Add QTPATH environment variable with path to Qt 4.7.4\bin
echo Add MINGWPATH environment variable with path to MinGW 4.4.0\bin
echo 7-Zip CommandLine is also required in PATH
echo.

:EOF