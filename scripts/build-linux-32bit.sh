#!/bin/bash

./scripts/set-version.sh
qmake 'CONFIG+=m32'
make release
cp nsbuilder nsbuilder_pl.qm release/
cp release nsbuilder1-`cat SVN_VERSION_FILE`-linux32-bin
zip pack/nsbuilder1-`cat SVN_VERSION_FILE`-linux32-bin.zip nsbuilder1-`cat SVN_VERSION_FILE`-linux32-bin/*