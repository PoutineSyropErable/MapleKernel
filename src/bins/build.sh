#!/usr/bin/env bash

nm ../build/myos_s.elf | grep " T " >nm_result.txt

python test.py nm_result.txt

gcc ./load_sym.c -o load_sym

./load_sym
