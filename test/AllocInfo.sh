#!/bin/bash
set -e
set -o pipefail

LD_PRELOAD=$pointerToAllocMap "$1" 2> "$2"
cat "$2"
cat "$2" | grep -F "x =" | grep -F " -> 1337|"
cat "$2" | grep -F "x + 3 =" | grep -F " -> 1337|"
cat "$2" | grep -F "x - 1 =" | grep -F " -> 0|"
cat "$2" | grep -F "x(free'd) =" | grep -F " -> 0|"
