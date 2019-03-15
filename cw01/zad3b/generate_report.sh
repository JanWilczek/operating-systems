#!/bin/bash
OUTPUT_FILE_NAME=raport3b.txt

make clean
rm -f $OUTPUT_FILE_NAME

OPTIMIZE_LEVEL[0]=-O0
OPTIMIZE_LEVEL[1]=-O3
OPTIMIZE_LEVEL[2]=-Os

for i in 0 1 2
do
    make speed_test_static CFLAGS=${OPTIMIZE_LEVEL[i]}
    ./speed_test_static $OUTPUT_FILE_NAME `echo "Static library, ${OPTIMIZE_LEVEL[i]}:"`

    make speed_test_shared CFLAGS=${OPTIMIZE_LEVEL[i]}
    ./speed_test_shared $OUTPUT_FILE_NAME "Shared library, ${OPTIMIZE_LEVEL[i]}:"

    make speed_test_dynamic CFLAGS=${OPTIMIZE_LEVEL[i]}
    ./speed_test_dynamic $OUTPUT_FILE_NAME "Dynamically loaded library, ${OPTIMIZE_LEVEL[i]}:"
done