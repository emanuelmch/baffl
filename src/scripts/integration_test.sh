#!/bin/sh

RESULT=0
TEMP_DIR=$(mktemp -d -t baffl-integration-test-XXXXXXXX)

COMPILER=$1
TEST_FILE=$2
TEST_OBJECT=${TEMP_DIR}/object.o
TEST_EXECUTABLE=${TEMP_DIR}/executable

TEST_EXPECTED_RESULT=$(cat "${TEST_FILE}.result")

# Compile the test file
$COMPILER "${TEST_FILE}" -o "${TEST_OBJECT}" || exit 1

# TODO: Stop relying on clang for linking
clang "${TEST_OBJECT}" -o "${TEST_EXECUTABLE}"
$TEST_EXECUTABLE
TEST_ACTUAL_RESULT=$?

if [ "$TEST_ACTUAL_RESULT" -eq "$TEST_EXPECTED_RESULT" ] ; then
  echo "Success: $TEST_ACTUAL_RESULT"
  RESULT=0
else
  echo "Failed: Expected $TEST_EXPECTED_RESULT, got $TEST_ACTUAL_RESULT"
  RESULT=1
fi

rm -rf $TEMP_DIR
exit $RESULT
