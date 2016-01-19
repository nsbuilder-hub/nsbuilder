#!/bin/bash

git clone https://github.com/nsbuilder-hub/nsbuilder.git
cd nsbuilder
./scripts/set-version.sh
cd ..
zip -r nsbuilder1-`cat nsbuilder/SVN_VERSION_FILE`-src.zip nsbuilder
rm -rf nsbuilder

