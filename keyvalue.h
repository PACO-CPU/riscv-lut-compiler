#ifndef RISCV_LUT_COMPILER_KEYVALUE_H
#define RISCV_LUT_COMPILER_KEYVALUE_H
#include <alpha/alpha.h>

/** Represents a single key-value.
  */
struct keyvalue_t {
  enum kind_t {
    String =0,
    Integer,
    Float,
  };
  alp::string name;
  kind_t kind;
  union {
    char *str;
    int64_t *inum;
    double  *fnum;
  };

};

#endif
