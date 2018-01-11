#!/bin/bash
if [ "$#" -ne 1 ]; then
    echo "Usage test.sh testdirname"
    exit
fi
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

source $DIR'/assert.sh'

for file in $DIR/$1/test*.i; do
    TEST_NAME=`basename $file .i`
    $COMPILER < $file 2>/dev/null >$DIR/tmp
    OUTPUT=`
    EXPECTED_FILE=${file::(-2)}".o"
    EXPECTED=`cat $EXPECTED_FILE`
    test "$TEST_NAME" "$OUTPUT" "$EXPECTED"
done