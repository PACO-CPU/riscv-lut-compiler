#!/usr/bin/env python3

import os,sys
import subprocess as sp

CXXFLAGS=["-std=c++11", "-I."]


objects=[]
for root,dirs,files in os.walk("."):
  
  for fn in files:
    fn=os.path.join(root,fn)
    (fc,fe)=os.path.splitext(fn)
    fe=fe.lower()

    if fe in { ".c", ".cpp", ".cc" }:
      obj="%s.o"%fc
      p=sp.Popen(
        ["gcc"]+CXXFLAGS+["-MM","-MT",obj,fn],
        stdout=sp.PIPE,stderr=sp.PIPE)
      (sout,serr)=p.communicate()
      sys.stdout.write(sout.decode("utf8"))
      sys.stderr.write(serr.decode("utf8"))
      if p.returncode!=0:
        sys.exit(p.returncode)
      
      objects.append(obj)

print("OBJ+=%s"%(" ".join(objects)))
