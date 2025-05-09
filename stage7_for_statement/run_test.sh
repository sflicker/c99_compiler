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

# compile C to ASM
echo "Compile..."
if ! ./$PROG "$SRC"; then
    echo "Compiler crash or error."
    exit 99
fi

echo "Assemble..."
if ! nasm -f elf64 "$ASM_FILE" -o "$OBJ_FILE"; then
    echo "Assembler failure."
    exit 98
fi

echo "Linking..."    
if ! gcc -no-pie -o "$EXE_FILE" "$OBJ_FILE"; then
    echo "Linking failure."
    exit 97
fi

echo "executing compiled file $EXE_FILE ..."
set +e
timeout 2s ./"$EXE_FILE"
EXIT_CODE=$?

if [ $EXIT_CODE -eq 124 ]; then
    echo -e "${RED}⏱ Timeout: $EXE_FILE exceeded time limit${NC}"
    exit 96
fi

set -e

echo "Program $EXE_FILE exited with code $EXIT_CODE"

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'



if [ $EXIT_CODE -eq $UNSIGNED_EXPECTED ]; then
    echo -e "${GREEN}✅ Test: $SRC Passed${NC}"
    exit 0
else 
    echo -e "${RED}❌ Test: $SRC Failed: expected $EXPECTED, got $EXIT_CODE"
    exit 1
fi
