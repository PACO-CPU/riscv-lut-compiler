#ifndef RISCV_LUT_COMPILER_UTIL_H
#define RISCV_LUT_COMPILER_UTIL_H

#include <iostream>
#include <istream>
#include <streambuf>
#include <string>

struct membuf_read_t : std::basic_streambuf<char>
{
    membuf_read_t(const char* begin, size_t cb) {
        this->setg((char*)begin, (char*)begin, (char*)begin+cb);
    }
};

struct target_type_t {
  enum base_type_t {
    Float,
    Int,
    UInt
  };

  base_type_t base;
  /** Number of bits of the type */
  int         width;

  bool set(const char *name) {
    #define OPT(n,b,w) else if (strcmp(name,n)==0) { base=b; width=w; }
    if (0) { }
    OPT("double",Float,64)
    OPT("float",Float,32)
    OPT("int",Int,32)
    OPT("uint",UInt,32)
    OPT("u64",Int,64)
    OPT("s64",Int,64)
    OPT("u32",Int,32)
    OPT("s32",Int,32)
    OPT("u16",Int,16)
    OPT("s16",Int,16)
    OPT("u8",Int,8)
    OPT("s8",Int,8)
    else return false;
    return true;

    #undef OPT
  }


};

#endif
