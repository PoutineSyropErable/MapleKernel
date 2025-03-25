#!/bin/bash

printf "Check the file and call one of these!\n\n"
option1="ndisasm -b 16 hello.bin"
option2="objdump -D -b binary -m i386 hello.bin"
option3='r2 -a x86 -b 16 -m 0x7c00 -n -c "aaa; pd 20" hello.bin'

printf "Option 1: %s\n" "$option1"
printf "Option 2: %s\n" "$option2"
printf "Option 3: %s\n" "$option3"

exit 0
ndisasm -b 16 hello.bin
objdump -D -b binary -m i386 hello.bin
r2 -a x86 -b 16 -m 0x7c00 -n -c "aaa; pd 20" hello.bin
#radare2 ^
