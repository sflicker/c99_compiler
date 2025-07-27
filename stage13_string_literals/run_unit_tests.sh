#!/bin/bash
echo "Running unit tests..."
pass=0
fail=0
total=0
failed_tests=()

GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[1;34m'
CYAN='\033[1;36m'
NC='\033[0m' # No Color

for test in unit_tests/build/*; do
    testname=$(basename "$test")
    echo ""
    echo -e "${BLUE}Running $testname${NC}"
    ((total++))
    if "$test"; then
        echo -e "${GREEN}✅ [PASS] $testname${NC}"
        ((pass++))
    else
        echo -e "${RED}💥 [FAIL] $testname${NC}"
        failed_tests+=("$testname")
        ((fail++))
    fi 
done

echo ""
echo "Summary: $total total, $pass passed, $fail failed"
if [ "$fail" -ne 0 ]; then
    echo -e "${RED}💥 Failed Tests${NC}"
    for T in "${failed_tests[@]}"; do
        echo " - $T"
    done
fi
exit $fail
