#!/bin/bash

cd build
svn checkout http://nsbuilder.googlecode.com/svn/trunk/ nsbuilder-read-only
cd nsbuilder-read-only
./scripts/ustaw-wersje-svn.sh
qmake
make
cp nsbuilder nsbuilder_pl.qm ../../nsbuilder
cd ../..
zip nsbuilder1-`cat build/nsbuilder-read-only/SVN_VERSION_FILE`-linux64-bin.zip nsbuilder/*
rm -rf build/nsbuilder-read-only




