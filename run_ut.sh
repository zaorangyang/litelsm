#!/bin/bash
for test_executable in $(find . -name "*_test"); do
    echo "Running tests in ${test_executable} ..."
    ${test_executable}
    echo ""
done
