#!/bin/bash

date +%Y%m%d-r`git log -1 --format=%h` > SVN_VERSION_FILE
