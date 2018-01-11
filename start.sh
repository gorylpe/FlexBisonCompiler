#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )";
make
find . -type f -name 'test*.sh' -exec chmod +x {} \;
export COMPILER=$DIR/compiler

#$DIR/tests/test.sh "read"
#$DIR/tests/test.sh "comment"
#$DIR/tests/test.sh "var"
$DIR/tests/test.sh "const_expr"
