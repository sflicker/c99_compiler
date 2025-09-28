#! /bin/sh

find tests/ -maxdepth 1 -type f ! -name '*.*' -exec rm -f {} +
rm -rf tests/*.o
rm -rf tests/*.s

