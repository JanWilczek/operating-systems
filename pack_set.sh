#!/bin/bash

SET_NAME=$(basename `exec pwd`)

mkdir -p "WilczekJan/$SET_NAME"
cp -R zad*/ "WilczekJan/$SET_NAME/"
tar cvzf "WilczekJan-$SET_NAME.tar.gz" WilczekJan
rm -R WilczekJan/
