#!/bin/bash/

# make test suites
make

echo "Expected runtime: 1.5 minutes"
# run key tests
time ./key_tests
# run padding tests
time ./pad_tests

# clean up
make clean
