#!/usr/bin/env bash

set -eou pipefail

zig build-exe better.zig -femit-bin="better_in_zig"

printf -- "\n\n=====Executing=====\n\n"

./better_in_zig
