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

all_passed=true

for cfile in tests/test*.c; do
    echo "----------------------------"
    echo "🔍 Running test for $cfile..."
    
    # assuming all tests should have an exit code of 42 to pass for now
    if ./run_test.sh "$cfile" 42 $PROG; then
        echo -e "${GREEN}✅ $cfile passed${NC}"
    else
        echo -e "${RED}❌ $cfile failed${NC}"
        all_passed=false
    fi

done

if [ "$all_passed" = true ]; then
    echo -e "${GREEN}🏆 All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}🔥 Some tests failed.${NC}"
    exit 1
fi