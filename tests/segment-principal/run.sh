#!/usr/bin/env bash

echo -e "\x1b[34;1msegmentation test: principal\x1b[30;0m"
function runnamed() {
  ../../riscv-lut-compiler -i --arch $1.arch $1.input
  colordiff $1.lut $1.lut.cmp
}

runnamed test1
runnamed test2
