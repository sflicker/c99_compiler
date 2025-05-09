#! /bin/bash

set +e    # don't exit on first failure

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

PROG=$1

echo "Cleaning previous runs..."
rm -f tests/*.s tests/*.o


for bin in tests/test*.c; do
    base="${cfile%.*}"
    if [ -f "$base" ]; then
        echo "Deleting binary $base"
        rm -f "$base"
    fi
done

echo "Running all tests..."

TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TOTAL=0
FAILED_TESTS=()
FAILURES=0

for cfile in tests/test*.c; do
    TOTAL_TESTS=$((TOTAL_TESTS + 1))

    filename=$(basename "$cfile")
    testname="${filename%%__*}"
    remainder="${filename#*__}"
    expected="${remainder%.c}"

    echo "----------------------------"
    echo "🔍 Running test for $testname (expecting $expected)..."
    
    # assuming all tests should have an exit code of 42 to pass for now
    OUTPUT=$(./run_test.sh "$cfile" "$expected" $PROG)
    STATUS=$?

    echo "STATUS: $STATUS"

    if [ "$STATUS" -eq 0 ]; then
        echo -e "${GREEN}✅ $testname passed${NC}"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    elif [ "$STATUS" -eq 99 ]; then
        echo -e "${RED}💥 $testname: compiler crash${NC}"
        FAILED_TOTAL=$((FAILED_TOTAL + 1))
        FAILED_TESTS+=("$testname (compiler crash)")
    elif [ "$STATUS" -eq 98 ]; then
        echo -e "${RED}💥 $testname: assembler failure${NC}"
        FAILED_TOTAL=$((FAILED_TOTAL + 1))
        FAILED_TESTS+=("$testname (assembler failure)")
    elif [ "$STATUS" -eq 97 ]; then
        echo -e "${RED}💥 $testname: linker failure${NC}"
        FAILED_TOTAL=$((FAILED_TOTAL + 1))
        FAILED_TESTS+=("$testname (Linker Failure)")
    elif [ "$STATUS" -eq 96 ]; then
        echo -e "${RED}⏱ $testname: timed out ${NC}"
        FAILED_TOTAL=$((FAILED_TOTAL + 1))
        FAILED_TESTS+=("$testname (timeout)")
    elif [ $STATUS -eq 1 ]; then
        echo -e "${RED}❌ $testname failed${NC}"
        FAILED_TOTAL=$((FAILED_TOTAL + 1))
        FAILED_TESTS+=("$testname (Wrong Output)")
    else 
        echo -e "${RED}❓ $testname failed with unknown status $STATUS${NC}"
        FAILED_TOTAL=$((FAILED_TOTAL + 1))
        FAILED_TESTS+=("$testname (unknown status $STATUS)")
    fi


done

echo "---------------------------------"
echo "📝 Test Summary:"
echo -e "${GREEN}✅ Passed: ${PASSED_TESTS}${NC}"
echo -e "${RED}❌ Failed: ${FAILED_TESTS}${NC}"
echo "🧮 Total tests: ${TOTAL_TESTS}"

if [ "$FAILED_TOTAL" -ne 0 ]; then
    echo -e "${RED}🔥 Some tests failed.${NC}"
    for T in "${FAILED_TESTS[@]}"; do
        echo " - $T"
    done
    exit 1
else
    echo -e "${GREEN}🏆 All tests passed!${NC}"
    exit 0
fi
