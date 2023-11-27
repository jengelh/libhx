#!/bin/sh
f="--analyze -Xanalyzer -analyzer-output=text -Wall -g3"
make -k check CC=clang CXX=clang++ CFLAGS="$f" CXXFLAGS="$f"
