#!/bin/bash
set -e
"$1" > "$2"
cat "$2"
cat "$2" | grep -qF "sanity check"
