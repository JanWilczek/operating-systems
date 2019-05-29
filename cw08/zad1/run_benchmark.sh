#!/bin/bash

make
rm Times.txt
for NUM_THREADS in 1 2 4 8
do
    for FILTER in test_filter.txt filter10.txt filter30.txt filter65.txt
    do
        for METHOD in block interleaved
        do
            ./imfilt -n $NUM_THREADS -d $METHOD -f ./test_data/apollonian_gasket.ascii.pgm -l ./test_data/$FILTER -o ./test_data/filtered_image.pgm >> Times.txt
        done
    done
done
