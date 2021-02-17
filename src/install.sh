#!/bin/bash
echo "copying dlls to " $1
mkdir -p $1
cp libx264.so.* $1 2>/dev/null || :
cp libx264.*.dylib $1 2>/dev/null || :
echo "copying lib to " $1
cp libx264.a $1 2>/dev/null || :

