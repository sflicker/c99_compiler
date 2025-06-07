#!/bin/bash
echo "Running unit tests..."
pass=0
fail=0

for test in unit_tests/build/*; do
    if "$test"; then
        echo "[PASS] $(basename "$test")"
        ((pass++))
    else
        echo "[FAIL] $(basename "$test")"
        ((fail++))
    fi 
done

echo ""
echo "Summary: $pass passed, $fail failed"
exit $fail
