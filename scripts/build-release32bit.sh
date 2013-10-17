#!/bin/bash

cd build
svn checkout http://nsbuilder.googlecode.com/svn/trunk/ nsbuilder-read-only
cd nsbuilder-read-only
./scripts/ustaw-wersje-svn.sh
qmake 'CONFIG+=m32'
make
cp nsbuilder nsbuilder_pl.qm ../../nsbuilder
cd ../..
zip nsbuilder1-`cat build/nsbuilder-read-only/SVN_VERSION_FILE`-linux32-bin.zip nsbuilder/*
rm -rf build/nsbuilder-read-only




