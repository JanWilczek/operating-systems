#!/bin/bash

# Check if an argument has been passed and if so, if it is a directory (should be)
if [ $# -eq 1 ] && [ -d $1 ]; then
    cd $1
fi

SET_NAME=$(basename `exec pwd`)

mkdir -p "WilczekJan/$SET_NAME"
cp -R zad*/ "WilczekJan/$SET_NAME/"
tar cvzf "WilczekJan-$SET_NAME.tar.gz" WilczekJan
rm -R WilczekJan/
