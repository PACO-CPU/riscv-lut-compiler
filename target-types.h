#ifndef RISCV_LUT_COMPILER_TARGET_FUNCTION_H
#define RISCV_LUT_COMPILER_TARGET_FUNCTION_H
#include "segment.h"


/** Function pointer prototype to use for the target function.
  *
  * To support both integer and double arguments and return values in the
  * target function, a wrapper is added to the code before it is compiled
  * and loaded into the LUT compiler tool. This is the signature of this
  * wrapper.
  *
  * We use call-by-reference here to avoid having to copy too much data. The
  * first argument being the actual input and the second one being the result.
  */
typedef void (*target_function_t)(const seg_data_t&,seg_data_t&);

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
    OPT("long",Int,64)
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
