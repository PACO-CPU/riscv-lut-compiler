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
#endif
