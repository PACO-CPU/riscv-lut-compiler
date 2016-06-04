#!/usr/bin/env python3
import subprocess as sp
import sys

Definitions=0
Rules=1
Code=2

state=Definitions

rulesSection=""

for ln in sys.stdin:
  if ln.strip()=="%%" and state<Code:
    if state==Rules:
      p=sp.Popen("gcc -E -P -",shell=True,stdout=sp.PIPE,stdin=sp.PIPE)
      (sout,serr)=p.communicate(input=rulesSection.encode("utf8"))
      if p.returncode!=0:
        sys.exit(1)
      sys.stdout.write(sout.decode("utf8"))
    sys.stdout.write(ln)
    state+=1
    continue

  if state==Rules:
    rulesSection+=ln
  else:
    sys.stdout.write(ln)

  
