#!/bin/bash
if [ "$#" -ne 1 ]; then
    echo "Usage test.sh testdirname"
    exit
fi
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

source $DIR'/../assert.sh'

for file in $DIR/$1/*.i; do
    TEST_NAME=`basename $file .i`
    $COMPILER < $file 2>/dev/null >$DIR/tmp
    OUTPUT=`$DIR/interpreter-cln $DIR/tmp < ${file::(-2)}".d" | tail -n+4 | head -n-1`
    rm $DIR/tmp
    EXPECTED_FILE=${file::(-2)}".o"
    EXPECTED=`cat $EXPECTED_FILE`
    test "$TEST_NAME" "$OUTPUT" "$EXPECTED"
done