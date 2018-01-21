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
    OUTPUTALL=`$DIR/interpreter-cln $DIR/tmp < ${file::(-2)}".d" | tail -n+4 > $DIR/output`
    OUTPUT=`cat $DIR/output | head -n-1`
    TIME=`cat $DIR/output | tail -n1`
    TIME=${TIME//[!0-9]/}
    rm $DIR/tmp
    rm $DIR/output
    EXPECTED_FILE=${file::(-2)}".o"
    EXPECTED=`cat $EXPECTED_FILE`
    test "$TEST_NAME" "$OUTPUT" "$EXPECTED"
    LAST_TIME=`cat ${file::(-2)}".t"`
    echo "Last time: $LAST_TIME"
    echo "New time:  $TIME"
done
