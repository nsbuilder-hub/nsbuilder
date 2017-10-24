NSBuilder
=========

![NSBuilder logo](https://raw.githubusercontent.com/nsbuilder-hub/nsbuilder/master/images/nsbuilder96.png)

NSBuilder is an cross-platform application for creating and executing 
Nassi–Shneiderman diagrams useful in introduction to programming.

## Features
* running diagrams or going step-by-step through them,
* tracing values of variables during execution,
* creation of schemas, procedures and functions,
* automatic diagrams validation,
* exporting diagrams in PNG or SVG formats,
* cross-platform (Windows and Linux).

## History
NSBuilder was created by [Aleksander Wojdyga](http://github.com/wojdyga)
during his work in the [Institute of Computer Science](http://cs.pollub.pl) of
[Lublin University of Technology](http://pollub.pl) for Theoretical Computer Science
Fundamentals course (TPI) and is still use for this course at the University.

Project is created using C++ language with a Qt library, version 4.7.4.

## Licensing
NSBuilder is licensed under GNU GPL version 2, allowing you to run, copy, modify,
and play without any requirements apart from sharing your changes (if any).

## Contributing
If you have found any issue with the NSBuilder, feel free to create an issue.
If you have made any modifications to the NSBuilder, we would be happy to include
them in the mainstream release if they align well with project goals. We are
open to any Pull Requests you possibly will be sending.

We are still under moving the project to GitHub and releasing it in a more open
way than it was before. More information and documentation are to come soon.

## Building from source

### Windows
To build on Windows you need QT version [4.7.4](http://download.qt.io/archive/qt/4.7/qt-win-opensource-4.7.4-mingw.exe) 
or [4.8.5](http://download.qt.io/archive/qt/4.8/4.8.5/qt-win-opensource-4.8.5-mingw.exe)
(both were tested) and MinGW compiler version 4.4.0, as well as GNU bison (version 2.4.1
tested) and flex (2.5.4 version tested).

Go to root directory and set 3 environmental variables, for example:
```batch
set QTPATH=C:\Qt\4.8.5\bin
set MINGWPATH=E:\nsbuilder-tools\mingw\bin
rem GNU bison, flex, date, iconv, m4 exe files should be in %NSBUILDENV%\bin
set NSBUILDENV=E:\nsbuilder-tools
```
Then, you should run `scripts\set-version.cmd`, which creates one of the files
describing current version, needed in build process.

Next, you can run `scripts\build-windows.bat` which automatically builds release
version of 32-bit NSBuilder, and puts resulting files in `release\` directory.

The `scripts\pack-windows.bat` uses 7-Zip command line to prepare "portable"
package of the application, while `scripts\pack-windows-setup.bat` prepares
an Inno Setup installer (Inno Setup 5 required).

The whole set of binaries required for building NSBuilder under Windows will be
provided in the coming weeks. 

### Linux
On Ubuntu 14.04 you have to install required packages (QT4, Bison, Flex, compilers 
and git). Tested QT version was 4.8.6 and GCC 4.8. There is a need for older Bison
(2.x), which can be installed from older packages.

```bash
# installing QT4 and compilers
sudo apt install git qt4-dev-tools gcc g++ make flex zip
# install bison version 2.7
wget http://launchpadlibrarian.net/140087283/libbison-dev_2.7.1.dfsg-1_amd64.deb
wget http://launchpadlibrarian.net/140087282/bison_2.7.1.dfsg-1_amd64.deb
sudo dpkg -i libbison-dev_2.7.1.dfsg-1_amd64.deb 
sudo dpkg -i bison_2.7.1.dfsg-1_amd64.deb
```

To compile release version:

```bash
./scripts/set-version.sh
qmake
make

lrelease nsbuilder.pro #for build translations
```

Or just run `scripts/build-linux-64bit.sh` to get executable version zipped to 
`pack` directory, where build artefacts are being copied.

There is also a 32-bit build script, which is not supported anymore.
