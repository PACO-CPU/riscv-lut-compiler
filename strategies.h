#ifndef RISCV_LUT_COMPILER_STRATEGIES_H
#define RISCV_LUT_COMPILER_STRATEGIES_H
#include "strategy-def.h"
#include "lut.h"
#include "weights.h"
#include "options.h"
#include <assert.h>

namespace segment_strategy {
  struct record_t {
    typedef void (*optimize_t) (
      LookupTable *lut, WeightsTable *weights, const options_t &options,
      uint32_t max_count);

    typedef uint32_t (*subdivide_t) (
      LookupTable *lut, WeightsTable *weights, const options_t &options,
      uint32_t first, uint32_t last, uint32_t max_count);

    subdivide_t subdivide;
    optimize_t  optimize;

    void execute(
      LookupTable *lut, WeightsTable *weights, options_t &options) const;
  };

  #define SEGMENT_STRATEGY(id,name) extern const record_t id;
  #include "strategy-decl.h"
  #undef SEGMENT_STRATEGY

  const record_t *get(id_t id_in); 
} // namespace segment_strategy

namespace approx_strategy {
  
  struct record_t {
    typedef void (*handle_segment_t) (
      LookupTable *lut, WeightsTable *weights, const options_t &options, 
      const segment_t &seg, seg_data_t &y0, seg_data_t &y1);
    
    handle_segment_t handle_segment;

    void execute(
      LookupTable *lut, WeightsTable *weights, options_t &options) const;
  };

  #define APPROX_STRATEGY(id,name) extern const record_t id;
  #include "strategy-decl.h"
  #undef APPROX_STRATEGY
  
  const record_t *get(id_t id_in); 

} // namespace approx_strategy

#endif
