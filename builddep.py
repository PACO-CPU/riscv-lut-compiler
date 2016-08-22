#!/usr/bin/env python3

import os,sys
import subprocess as sp

CXXFLAGS=["-std=c++11", "-I."]

def get_subprocess_returncode(sub_command):
    child = sp.Popen(sub_command, stdout=sp.PIPE)
    data = child.communicate()[0]
    return int(child.returncode)

def lua_exists():
    # first try just lua
    pkg_string = ["pkg-config", "--exists"] + ["lua"]
    if get_subprocess_returncode(pkg_string) == 0:
        return "lua"
    # try versions 5.2 - 5.3
    for i in range(2,3):
        pkg_string[2] = "lua5." + str(i)
        if get_subprocess_returncode(pkg_string) == 0:
           return "lua5." + str(i)
    return None

def lua_version():
    lua_version = lua_exists()

    if lua_version == None:
        print("Lua was not found on your system. Please install the lua-dev package.")
        exit(1)
    return str(lua_version)

def gen_lib_ldflags():
    print("LDFLAGS+=$(shell pkg-config --libs " + lua_version() + ") -ldl")

def gen_lib_cxxflags():
    print("CXXFLAGS+=$(shell pkg-config --cflags " + lua_version() + ")")

objects=[]
tests=[]
for root,dirs,files in os.walk("."):
  for fn_base in files:
    fn=os.path.join(root,fn_base)
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
    elif fn.startswith("./tests/") and fn_base in { "run.sh" }:
      ident="test-%s"%os.path.basename(root)
      print("%s:\n\tcd %s && ./%s"%(ident,root,fn_base))
      tests.append(ident)

print("OBJ+=%s"%(" ".join(objects)))
print("TESTS+=%s"%(" ".join(tests)))

gen_lib_ldflags()
gen_lib_cxxflags()

