CD "%userprofile%\Pulpit"
set DATA=20130926
set /P VERSION=<nsbuilder1-%DATA%-src\nsbuilder1\SVN_VERSION_FILE

mkdir nsbuilder1-%DATA%-win-bin
copy nsbuilder-release-files nsbuilder1-%DATA%-win-bin
copy nsbuilder1-%DATA%-src\nsbuilder1\release\nsbuilder.exe nsbuilder1-%DATA%-win-bin
copy nsbuilder1-%DATA%-src\nsbuilder1\nsbuilder_pl.qm nsbuilder1-%DATA%-win-bin
copy nsbuilder1-%DATA%-src\nsbuilder1\index.html nsbuilder1-%DATA%-win-bin
copy nsbuilder1-%DATA%-src\nsbuilder1\scripts\nsbuilder2.iss nsbuilder1-%DATA%-win-bin
cd nsbuilder1-%DATA%-win-bin
"C:\Program Files\Inno Setup 5\ISCC.exe" /dMyAppVersion=%VERSION% "nsbuilder2.iss"
cd ..
copy nsbuilder1-%DATA%-win-bin\Output\setup.exe nsbuilder1-%DATA%-setup.exe
