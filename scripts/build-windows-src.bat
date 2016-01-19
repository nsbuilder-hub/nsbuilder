@echo off
git clone https://github.com/nsbuilder-hub/nsbuilder.git
cd nsbuilder
call scripts\set-version.bat
cd ..
SET /P VERSION=<nsbuilder\SVN_VERSION_FILE
7z a nsbuilder1-%VERSION%-src.zip nsbuilder
del /s /q nsbuilder
rmdir /s /q nsbuilder
move nsbuilder1-%VERSION%-src.zip ..\pack