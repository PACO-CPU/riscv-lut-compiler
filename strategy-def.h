#ifndef RISCV_LUT_COMPILER_STRATEGY_DEF_H
#define RISCV_LUT_COMPILER_STRATEGY_DEF_H


namespace segment_strategy {
  enum id_t {
    INVALID=0,
    #define SEGMENT_STRATEGY(id,name) ID_##id,
    #include "strategy-decl.h"
    #undef SEGMENT_STRATEGY
  };
};

namespace approx_strategy {
  enum id_t {
    INVALID=0,
    #define APPROX_STRATEGY(id,name) ID_##id,
    #include "strategy-decl.h"
    #undef APPROX_STRATEGY
  };
};

#endif
