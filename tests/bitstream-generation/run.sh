#!/usr/bin/env bash

echo -e "\x1b[34;1mapproximation test: linear\x1b[30;0m"
function runnamed() {
  ../../riscv-lut-compiler -i --arch $1.arch $1.input -g
  colordiff $1.lut $1.lut.cmp
  ../../riscv-lut-compiler -c -D --arch $1.arch $1.lut
  colordiff $1.dump $1.dump.cmp
  riscv-lut-tool -a $1.arch -d $1.dump -s 0 16777215 32768 $1.sim.dat
}

runnamed test1
