#! /bin/sh

stage_dir=.

find "$stage_dir" -type d -name "CMakeFiles" -exec rm -rf {} +
rm -f "$stage_dir"/CMakeCache.txt
rm -f "$stage_dir"/cmake_install.cmake
rm -f "$stage_dir"/Makefile
rm -f "$stage_dir"/compile_Commands.jso
rm -rf "$stage_dir"/build

find "$stage_dir" -type f \( -name "cmake-build-debug" -o -name "*.o" -o -name "*.out" -o -name "core*" -o -name "*.out"  \) -delete
