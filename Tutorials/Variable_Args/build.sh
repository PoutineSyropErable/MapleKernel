#!/usr/bin/env bash

gcc -no-pie push_var_args.c -o push_var_args -g -O0

./push_var_args
