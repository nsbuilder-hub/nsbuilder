#!/bin/bash

./scripts/set-version.sh
qmake 'CONFIG+=m32'
make clean
make
lrelease nsbuilder.pro #translations

if [ $? -eq 0 ]; then
	rm -rf release
	mkdir release
	cp nsbuilder nsbuilder_pl.qm release/
	cp release nsbuilder1-`cat SVN_VERSION_FILE`-linux86-bin
	if [ ! -e pack ]; then mkdir pack; fi
	zip pack/nsbuilder1-`cat SVN_VERSION_FILE`-linux86-bin.zip release/*
fi
