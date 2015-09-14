#!/bin/sh
set -x -e
g++ test.cpp -march=native -std=gnu++11 -O2 -o test -lpthread
