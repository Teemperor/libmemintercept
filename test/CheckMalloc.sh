#!/bin/bash
set -e
set -o pipefail

LD_PRELOAD=$printFrees "$1" 2> "$2"
cat "$2"
cat "$2" | grep -F "malloc(1337)"
cat "$2" | grep -F "realloc(" | grep -F "1377)"
cat "$2" | grep -F "malloc(7331)"
