#ifndef RISCV_LUT_COMPILER_STRATEGIES_H
#define RISCV_LUT_COMPILER_STRATEGIES_H
#include "strategy-def.h"
#include "lut.h"
#include "weights.h"
#include "options.h"
#include <assert.h>

namespace segment_strategy {
  struct record_t {
    typedef void (*execute_t) (
      LookupTable *lut, WeightsTable *weights, options_t &options);

    execute_t execute;
  };

  #define SEGMENT_STRATEGY(id,name) extern const record_t id;
  #include "strategy-decl.h"
  #undef SEGMENT_STRATEGY

  const record_t *get(id_t id_in); 
} // namespace segment_strategy

namespace approx_strategy {
  
  struct record_t {
    typedef void (*execute_t) (
      LookupTable *lut, WeightsTable *weights, options_t &options);

    execute_t execute;
  };

  #define APPROX_STRATEGY(id,name) extern const record_t id;
  #include "strategy-decl.h"
  #undef APPROX_STRATEGY
  
  const record_t *get(id_t id_in); 

} // namespace approx_strategy

#endif
