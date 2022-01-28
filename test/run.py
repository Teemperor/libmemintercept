#!/usr/bin/env python

import os
import sys
import subprocess

args = list(sys.argv[:])

env_var_str = args[1]
for env_var in env_var_str.split(":"):
  if env_var.strip() == "":
    continue
  split_var = env_var.split("=")
  os.environ[split_var[0]] = split_var[1]

new_args = ["bash", "-x"] + args[2:]

sys.exit(subprocess.run(new_args).returncode)
