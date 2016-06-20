#!/usr/bin/env bash

echo -e "\x1b[34;1msegmentation test: principal\x1b[30;0m"
../../riscv-lut-compiler -i test1.input
colordiff test1.lut test1.lut.cmp

../../riscv-lut-compiler -i test2.input
colordiff test2.lut test2.lut.cmp
