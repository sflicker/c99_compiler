#! /bin/bash

set -e

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

PROG=$1

echo "Cleaning previous runs..."
rm -f tests/*.s tests/*.o


for bin in tests/test*.c; do
    base="${cfile%.*}"
    if [ =f "$base" ]; then
        echo "Deleting binary $base"
        rm -f "$base"
    fi
done

echo "Running all tests..."

total_tests=0
passed_tests=0
failed_tests=0

for cfile in tests/test*.c; do
    total_tests=$((total_tests + 1))

    filename=$(basename "$cfile")
    testname="${filename%%__*}"
    remainder="${filename#*__}"
    expected="${remainder%.c}"

    echo "----------------------------"
    echo "🔍 Running test for $testname (expecting $expected)..."
    
    # assuming all tests should have an exit code of 42 to pass for now
    if ./run_test.sh "$cfile" "$expected" $PROG; then
        echo -e "${GREEN}✅ $testname passed${NC}"
        passed_tests=$((passed_tests + 1))
    else
        echo -e "${RED}❌ $testname failed${NC}"
        failed_tests=$((failed_tests + 1))
    fi

done

echo "---------------------------------"
echo "📝 Test Summary:"
echo -e "${GREEN}✅ Passed: ${passed_tests}${NC}"
echo -e "${RED}❌ Failed: ${failed_tests}${NC}"
echo "🧮 Total tests: ${total_tests}"

if [ "$failed_tests" -eq 0 ]; then
    echo -e "${GREEN}🏆 All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}🔥 Some tests failed.${NC}"
    exit 1
fi