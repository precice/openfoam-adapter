#!/bin/sh

# Format all C++ files with clang-format
find . \( -iname "*.H" -o -iname "*.C" \) -exec clang-format -i {} \;