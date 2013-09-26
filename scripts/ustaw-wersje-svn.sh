#!/bin/bash

date +%Y%m%d-r`svn  info | grep Wersja | cut -f 2 -d ' '` > SVN_VERSION_FILE
