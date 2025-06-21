#! /bin/bash

set +e    # don't exit on first failure

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

USE_VALGRIND=0

#parse optional --memcheck flag
if [ "$1" == "--memcheck" ]; then
    USE_VALGRIND=1
    shift
fi

PROG=$1

echo "Cleaning previous runs..."
rm -rf integration_tests/build
#rm -f tests/*.s tests/*.o


#for bin in tests/test*.c; do
#    base="${cfile%.*}"
#    if [ -f "$base" ]; then
#        echo "Deleting binary $base"
#        rm -f "$base"
#    fi
#done

mkdir -p integration_tests/build

echo "Running all tests..."

TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TOTAL=0
FAILED_TESTS=()
FAILURES=0
COMPILER_CRASH_TOTAL=0
MEMORY_LEAK_TOTAL=0
ASSEMBLER_FAILURE_TOTAL=0
LINKER_FAILURE_TOTAL=0
TIMEOUT_TOTAL=0
WRONG_OUTPUT_TOTAL=0
UNKNOWN_FAILURE_TOTAL=0

for cfile in integration_tests/test*.c; do
    echo $cfile
    TOTAL_TESTS=$((TOTAL_TESTS + 1))

    filename=$(basename "$cfile")
    testname="${filename%%__*}"
    remainder="${filename#*__}"
    expected="${remainder%.c}"

    echo ""
    echo "----------------------------"
    echo ""
    echo "🔍 Running test for $testname (expecting $expected)..."
    
    # assuming all tests should have an exit code of 42 to pass for now
    if [ $USE_VALGRIND -eq 1 ]; then
        OUTPUT=$(./run_test.sh --memcheck "$cfile" "$expected" $PROG)
        STATUS=$?
    else 
        OUTPUT=$(./run_test.sh "$cfile" "$expected" $PROG)
        STATUS=$?
    fi

    echo "STATUS: $STATUS"

    if [ "$STATUS" -eq 0 ]; then
        echo -e "${GREEN}✅ $testname passed${NC}"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    elif [ "$STATUS" -eq 99 ]; then
        echo -e "${RED}💥 $testname: compiler crash${NC}"
        FAILED_TOTAL=$((FAILED_TOTAL + 1))
        FAILED_TESTS+=("$testname (compiler crash)")
        COMPILER_CRASH_TOTAL=$((COMPILER_CRASH_TOTAL + 1))
    elif [ "$STATUS" -eq 98 ]; then
        echo -e "${RED}💥 $testname: assembler failure${NC}"
        FAILED_TOTAL=$((FAILED_TOTAL + 1))
        FAILED_TESTS+=("$testname (assembler failure)")
        ASSEMBLER_FAILURE_TOTAL=$((ASSEMBLER_FAILURE_TOTAL + 1))
    elif [ "$STATUS" -eq 97 ]; then
        echo -e "${RED}💥 $testname: linker failure${NC}"
        FAILED_TOTAL=$((FAILED_TOTAL + 1))
        FAILED_TESTS+=("$testname (Linker Failure)")
        LINKER_FAILURE_TOTAL=$((LINKER_FAILURE_TOTAL + 1))
    elif [ "$STATUS" -eq 96 ]; then
        echo -e "${RED}⏱ $testname: timed out ${NC}"
        FAILED_TOTAL=$((FAILED_TOTAL + 1))
        FAILED_TESTS+=("$testname (timeout)")
        TIMEOUT_TOTAL=$((TIMEOUT_TOTAL + 1))
    elif [ "$STATUS" -eq 90 ]; then
        echo -e "${RED}💥 $testname: compiler memory leaks${NC}"
        FAILED_TOTAL=$((FAILED_TOTAL + 1))
        FAILED_TESTS+=("$testname (compiler memory leaks)")
        MEMORY_LEAK_TOTAL=$((MEMORY_LEAK_TOTAL + 1))
    elif [ $STATUS -eq 1 ]; then
        echo -e "${RED}❌ $testname failed${NC}"
        FAILED_TOTAL=$((FAILED_TOTAL + 1))
        FAILED_TESTS+=("$testname (Wrong Output)")
        WRONG_OUTPUT_TOTAL=$((WRONG_OUTPUT_TOTAL + 1))
    else 
        echo -e "${RED}❓ $testname failed with unknown status $STATUS${NC}"
        FAILED_TOTAL=$((FAILED_TOTAL + 1))
        FAILED_TESTS+=("$testname (unknown status $STATUS)")
        UNKNOWN_FAILURE_TOTEL=$((UNKNOWN_FAILURE_TOTAL + 1))
    fi


done

echo "---------------------------------"
echo "📝 Test Summary:"
echo "🧮 Total tests: ${TOTAL_TESTS}"
echo -e "${GREEN}✅ Passed: ${PASSED_TESTS}${NC}"
echo -e "Subtotals..."
echo -e "    ${RED}❌ Wrong Output Total: ${WRONG_OUTPUT_TOTAL}"
echo -e "    ${RED}❌ Compiler Crash Total: ${COMPILER_CRASH_TOTAL}"
echo -e "    ${RED}❌ Memory Leak Total: ${MEMORY_LEAK_TOTAL}"
echo -e "    ${RED}❌ Assembler Failure Total: ${ASSEMBLER_FAILURE_TOTAL}"
echo -e "    ${RED}❌ Linker Failure Total: ${LINKER_FAILURE_TOTAL}"
echo -e "    ${RED}❌ Timeout Total: ${TIMEOUT_TOTAL}"
echo -e "    ${RED}❌ Unknown Failure Total: ${UNKNOWN_FAILURE_TOTAL}"
echo -e "${RED}❌ Failed: ${FAILED_TESTS}${NC}"

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
