#!/bin/bash
OUTPUT_FILE_NAME=raport3a.txt

make clean
rm -f $OUTPUT_FILE_NAME

make speed_test_static
./speed_test_static $OUTPUT_FILE_NAME "Static library:"

make speed_test_shared
./speed_test_shared $OUTPUT_FILE_NAME "Shared library:"

make speed_test_dynamic
./speed_test_dynamic $OUTPUT_FILE_NAME "Dynamically loaded library:"