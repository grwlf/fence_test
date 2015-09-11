#!/bin/sh
set -x -e
gcc test.cpp  -O0 -o test -lpthread
