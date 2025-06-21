
#! /bin/bash

USE_VALGRIND=0

#parse optional --memcheck flag
if [ "$1" == "--memcheck" ]; then
    USE_VALGRIND=1
    shift
fi

SRC=$1
EXPECTED=$2
PROG=$3

if [ -z "$SRC" ] || [ -z "$EXPECTED" ] || [ -z "$PROG" ]; then
    echo "Usage: $0 [--memcheck] source.c expected program"
    exit 1
fi

UNSIGNED_EXPECTED=$(( (EXPECTED + 256) % 256 ))

echo SRC=$SRC
echo EXPECTED=$EXPECTED
echo PROG=$PROG

filename=$(basename "$SRC" .c)
ASM_FILE="integration_tests/build/${filename}.s"
OBJ_FILE="integration_tests/build/${filename}.o"
EXE_FILE="integration_tests/build/${filename}"

#ASM_FILE="${SRC%.c}.s"
#OBJ_FILE="${SRC%.c}.o"
#EXE_FILE="${SRC%.*}"

echo ASM_FILE=$ASM_FILE
echo OBJ_FILE=$OBJ_FILE
echo EXE_FILE=$EXE_FILE

set -e

# compile C to ASM
CMD="./$PROG $SRC -o $ASM_FILE"
if [ "$USE_VALGRIND" -eq 1 ]; then
    echo "🔍 Running under Valgrind: $CMD"
    CMD="valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=2 $CMD"
fi

echo "Command: $CMD"

echo "Compile..."
if $CMD; then
    echo "✅ Compilation succeeded with no leaks"
else
    exit_code=$?

    if [ $exit_code -eq 1 ]; then
        echo "❌ Compiler crash or error. (exit 1)"
        exit 99
    elif [ $exit_code -eq 2 ]; then
        echo "❌ Valgrind detected memory errors (exit 2)"
        exit 90
    elif [ $exit_code -ne 0 ]; then
        echo "❌ Unexpected exit code: $exit_code"
        exit $exit_code
    fi
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
