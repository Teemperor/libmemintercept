#!/bin/bash
set -e
LD_PRELOAD=$printFrees "$1" 2> "$2"
cat "$2"
cat "$2" | grep -qF "malloc(1337)"
