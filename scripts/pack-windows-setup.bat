@echo off
IF /I "%NSBUILDENV%"=="" GOTO :BADENV

SET /P VERSION=<SVN_VERSION_FILE

copy scripts\nsbuilder2.iss .\release
cd release
"C:\Program Files (x86)\Inno Setup 5\ISCC.exe" /dMyAppVersion=%VERSION% "nsbuilder2.iss"
cd ..
copy release\Output\setup.exe pack\nsbuilder1-%VERSION%-setup.exe
del /s /q release\Output
rmdir release\Output

goto :EOF

:BADENV
echo Bad environment. Add NSBUILDENV environment variable with paths to:
echo     GnuWin32: date, bison, flex, libiconv, regex, libintl
echo Add QTPATH environment variable with path to Qt 4.7.4\bin
echo Add MINGWPATH environment variable with path to MinGW 4.4.0\bin
echo 7-Zip CommandLine is also required in PATH
echo.

:EOF