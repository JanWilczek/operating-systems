#!/bin/bash

echo $((`find -type f -name '*.c' -exec cat {} + | wc -l` + `find -type f -name '*.h' -exec cat {} + | wc -l`))
