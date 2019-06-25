#!/bin/bash/

# make test suites
make

# run key tests
time ./key_tests
# run padding tests
time ./pad_tests

# clean up
make clean
