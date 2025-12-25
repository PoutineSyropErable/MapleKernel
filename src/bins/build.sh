#!/usr/bin/env bash

nm ../build/myos_s.elf | grep " T " >nm_result.txt
