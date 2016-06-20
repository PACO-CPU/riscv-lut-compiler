#!/usr/bin/env bash

echo -e "\x1b[34;1msegmentation test: uniform\x1b[30;0m"
function runnamed() {
  ../../riscv-lut-compiler -i --arch $1.arch $1.input -g
  colordiff $1.lut $1.lut.cmp
}

runnamed test1
runnamed test2
