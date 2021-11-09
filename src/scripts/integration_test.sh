#!/bin/sh

TEMP_DIR=$(mktemp -d -t baffl-integration-test-XXXXXXXX)
clean_temp_dir() {
  rm -rf "$TEMP_DIR"
}
trap clean_temp_dir EXIT

COMPILER=$1
TEST_FILE=$2
TEST_OBJECT=${TEMP_DIR}/object.o
TEST_EXECUTABLE=${TEMP_DIR}/executable
TEST_OUTPUT_FILE=${TEMP_DIR}/output

# Step: Compile the test file
$COMPILER "${TEST_FILE}" -vo "${TEST_OBJECT}" || exit 1

# TODO: Stop relying on clang for linking
clang "${TEST_OBJECT}" -o "${TEST_EXECUTABLE}"
$TEST_EXECUTABLE >$TEST_OUTPUT_FILE
TEST_ACTUAL_RESULT=$?

# Step: Check for the file output (defaults to empty)
if [ -f "$TEST_FILE.output" ]; then
  #TODO: Find a way to make this echo unnecessary
  echo "" >>"$TEST_OUTPUT_FILE"
  if cmp -s "$TEST_FILE.output" "$TEST_OUTPUT_FILE"; then
    echo "Success: Output was correct"
  else
    echo "Failed: Expected output was [$(cat "$TEST_FILE.output")], actual output was [$(cat "$TEST_OUTPUT_FILE")]"
    exit 1
  fi
else # Output should be empty if there's no expected output
  if [ -s "$TEST_OUTPUT_FILE" ]; then
    echo "Failed: Expected no output, but there was one: [$(cat "$TEST_OUTPUT_FILE")]"
    exit 1
  else
    echo "Success: Output was correctly empty"
  fi
fi

# Step: Check for the file result (defaults to 0)
if [ -f "$TEST_FILE.result" ]; then
  TEST_EXPECTED_RESULT=$(cat "${TEST_FILE}.result")
else
  TEST_EXPECTED_RESULT=0
fi

if [ "$TEST_ACTUAL_RESULT" -eq "$TEST_EXPECTED_RESULT" ]; then
  echo "Success: $TEST_ACTUAL_RESULT"
else
  echo "Failed: Expected $TEST_EXPECTED_RESULT, got $TEST_ACTUAL_RESULT"
  exit 1
fi
