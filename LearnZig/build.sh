#!/usr/bin/env bash

set -eou pipefail

zig build-exe hello.zig

./hello
