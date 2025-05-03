#! /bin/bash

SRC=$1
EXPECTED=$2
PROG=$3

UNSIGNED_EXPECTED=$(( (EXPECTED + 256) % 256 ))

echo SRC=$SRC
echo EXPECTED=$EXPECTED
echo PROG=$PROG

ASM_FILE="${SRC%.c}.s"
OBJ_FILE="${SRC%.c}.o"
EXE_FILE="${SRC%.*}"

echo ASM_FILE=$ASM_FILE
echo OBJ_FILE=$OBJ_FILE
echo EXE_FILE=$EXE_FILE

set -e

./$PROG $SRC

echo "linking..."
nasm -f elf64 $ASM_FILE -o $OBJ_FILE
gcc -no-pie -o $EXE_FILE $OBJ_FILE

echo "executing compiled file..."
set +e
./"$EXE_FILE"
EXIT_CODE=$?
set -e
echo "finished running compiled file"

echo "Program '$EXE_FILE' exited with code $EXIT_CODE"

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

if [ $EXIT_CODE -eq $UNSIGNED_EXPECTED ]; then
    echo -e "${GREEN}✅ Test: $SRC Passed${NC}"
else 
    echo -e "${RED}❌ Test: $SRC Failed: expected $EXPECTED, got $EXIT_CODE"
    exit 1
fi
