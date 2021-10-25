#!/bin/sh

RESULT=0
TEMP_DIR=$(mktemp -d -t baffl-integration-test-XXXXXXXX)

COMPILER=$1
TEST_FILE=$2
TEST_OBJECT=${TEMP_DIR}/object.o
TEST_EXECUTABLE=${TEMP_DIR}/executable
TEST_OUTPUT_FILE=${TEMP_DIR}/output

echo $TEMP_DIR

# Compile the test file
$COMPILER "${TEST_FILE}" -vo "${TEST_OBJECT}" || exit 1

# TODO: Stop relying on clang for linking
clang "${TEST_OBJECT}" -o "${TEST_EXECUTABLE}"
$TEST_EXECUTABLE > $TEST_OUTPUT_FILE
TEST_ACTUAL_RESULT=$?

if [ -f "$TEST_FILE.output" ]; then
  #TODO: Find a way to make this echo unnecessary
  echo "" >> "$TEST_OUTPUT_FILE"
  if cmp -s "$TEST_FILE.output" "$TEST_OUTPUT_FILE"; then
    echo "Success: Output was correct"
  else
    echo "Failed: Expected output was [$(cat "$TEST_FILE.output")], actual output was [$(cat "$TEST_OUTPUT_FILE")]"
    #TODO: Stop having the cleanup + exit code here
#    rm -rf "$TEMP_DIR"
    exit 1
  fi
fi

if [ -f "$TEST_FILE.result" ]; then
  TEST_EXPECTED_RESULT=$(cat "${TEST_FILE}.result")
else
  TEST_EXPECTED_RESULT=0
fi # "${TEST_FILE}.result"

if [ "$TEST_ACTUAL_RESULT" -eq "$TEST_EXPECTED_RESULT" ] ; then
  echo "Success: $TEST_ACTUAL_RESULT"
  RESULT=0
else
  echo "Failed: Expected $TEST_EXPECTED_RESULT, got $TEST_ACTUAL_RESULT"
  RESULT=1
fi

rm -rf "$TEMP_DIR"
exit $RESULT
