#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )";
make clean
make
find . -type f -name 'test*.sh' -exec chmod +x {} \;
export COMPILER=$DIR/compiler

$DIR/tests/execution/test.sh "final-tests"
#$DIR/tests/execution/test.sh "labor4
#$DIR/tests/compilation/test.sh "read"
#$DIR/tests/compilation/test.sh "comment"
#$DIR/tests/compilation/test.sh "var"
#$DIR/tests/compilation/test.sh "const_expr"
