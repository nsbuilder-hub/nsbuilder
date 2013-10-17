#!/bin/bash

svn checkout http://nsbuilder.googlecode.com/svn/trunk/ nsbuilder
cd nsbuilder
./scripts/ustaw-wersje-svn.sh
cd ..
zip -r nsbuilder1-`cat nsbuilder/SVN_VERSION_FILE`-src.zip nsbuilder
rm -rf nsbuilder

