#ifndef RISCV_LUT_COMPILER_ARCH_CONFIG_H
#define RISCV_LUT_COMPILER_ARCH_CONFIG_H
#include <alpha/alpha.h>
#include <stdio.h>

struct arch_config_t {
  enum {
    Default_numSegments = 8,
  };
  // options go here.
  // todo: define more arguments depending on the LUT meta-architecture
  int numSegments;
  
  arch_config_t() : 
    numSegments(Default_numSegments)
    {
  }

  void parse(const char *ptr, size_t cb, const char *name);
  void parseFile(const char *fn);
  
};

#endif
