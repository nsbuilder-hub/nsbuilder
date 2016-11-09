@echo off
IF /I "%NSBUILDENV%"=="" GOTO :BADENV
SET FLAVOR=release
IF /I "%1"=="DEBUG" SET FLAVOR=debug

rem Set PATH with Qt, MinGW and additional tools we need
set PATH=%PATH%;%QTPATH%;%MINGWPATH%;%NSBUILDENV%\bin

call scripts\set-version.cmd
qmake "CONFIG+=win32"
make %FLAVOR%
lrelease nsbuilder.pro

rem Automatic makefile does not work somehow with bison generated files
rem so we are changing names here ourselves and calling make again

IF ERRORLEVEL 1 GOTO :FIXBISON

:PACK
IF /I "%FLAVOR%"=="DEBUG" GOTO :EOF

copy nsbuilder_pl.qm release\
copy %QTPATH%\QtCore4.dll release\
copy %QTPATH%\QtGui4.dll release\
copy %QTPATH%\QtNetwork4.dll release\
copy %QTPATH%\QtXml4.dll release\
copy %MINGWPATH%\mingwm10.dll release\
copy %MINGWPATH%\libgcc_s_dw2-1.dll release\
del release\qrc_nsbuilder.cpp

GOTO :EOF

:FIXBISON
echo Fixing BISON Qt error
ren imp.tab.h imp_yacc.h
ren imp.tab.c imp_yacc.cpp
make

GOTO :PACK

:BADENV
echo Bad environment. Add NSBUILDENV environment variable with paths to:
echo     GnuWin32: date, bison, flex, libiconv, regex, libintl
echo Add QTPATH environment variable with path to Qt 4.7.4\bin
echo Add MINGWPATH environment variable with path to MinGW 4.4.0\bin
echo 7-Zip CommandLine is also required in PATH
echo.

:EOF

