#!/bin/bash

# Run all .to files in the tests, if any return non-zero, unit tests failed
# This script is used to run all unit tests in the tests using the compiled ./main.exe

start=$(date +%s)

for f in *.to; do
    echo "Running $f"
    ../build/dosato "$f"
    if [ $? -ne 0 ]; then
        echo "Unit tests failed"
        echo "At: $f"
        exit 1
    fi
done

echo "All unit tests passed"
