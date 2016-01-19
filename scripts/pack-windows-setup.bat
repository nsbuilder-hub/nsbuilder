@echo off
IF /I "%NSBUILDENV%"=="" GOTO :BADENV

copy nsbuilder1-%DATA%-src\nsbuilder1\index.html nsbuilder1-%DATA%-win-bin
copy nsbuilder1-%DATA%-src\nsbuilder1\scripts\nsbuilder2.iss nsbuilder1-%DATA%-win-bin
cd nsbuilder1-%DATA%-win-bin
"C:\Program Files\Inno Setup 5\ISCC.exe" /dMyAppVersion=%VERSION% "nsbuilder2.iss"
cd ..
copy nsbuilder1-%DATA%-win-bin\Output\setup.exe nsbuilder1-%DATA%-setup.exe

goto :EOF

:BADENV
echo Bad environment. Add NSBUILDENV environment variable with paths to:
echo     GnuWin32: date, bison, flex, libiconv, regex, libintl
echo Add QTPATH environment variable with path to Qt 4.7.4\bin
echo Add MINGWPATH environment variable with path to MinGW 4.4.0\bin
echo 7-Zip CommandLine is also required in PATH
echo.

:EOF