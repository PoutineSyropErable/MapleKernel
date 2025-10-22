#!/usr/bin/env bash

nasm -f bin ./miniboot16.s -o boot.com
bochs -f bochsrc.txt
