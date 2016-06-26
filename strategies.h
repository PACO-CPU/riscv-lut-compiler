/** \file strategies.h
  * \brief Defines the common interface for using modular strategies from
  * the regular tool flow (which does not know of any strategies in particular).
  *
  * Strategies are declared in strategy-decl.h and defined in seperate source
  * code files by defining an instance of segment_strategy::record_t or
  * approx_strategy::record_t with the identifier set forth in strategy-decl.h.
  * These strategies are identified by an auto-generated numeric constant (enum)
  * derived from the strategies identifier. When specified as part of input 
  * files, the strategy gets identified by a string also defined in 
  * strategy-decl.h which gets translated to the corresponding numeric constant.
  *
  */
#ifndef RISCV_LUT_COMPILER_STRATEGIES_H
#define RISCV_LUT_COMPILER_STRATEGIES_H
#include "strategy-def.h"
#include "lut.h"
#include "weights.h"
#include "options.h"
#include <assert.h>

namespace segment_strategy {
  /** Type representing a single segmentation strategy.
    *
    * Strategies themselves are declared in strategy-decl.h and define a 
    * symbol according to the identifier defined therein. This symbol is of
    * type record_t and contains entry points into the methods handling the
    * respective strategy.
    */
  struct record_t {
    typedef void (*optimize_t) (
      LookupTable *lut, WeightsTable *weights, const options_t &options,
      uint32_t max_count);

    typedef uint32_t (*subdivide_t) (
      LookupTable *lut, WeightsTable *weights, const options_t &options,
      uint32_t first, uint32_t last, uint32_t max_count);
    
    /** Subdivision method. If specified, this strategy may be used by
      * specifying a range in segment space that needs to be subdivided into
      * a number of segments while not exceeding a maximum count.
      */
    subdivide_t subdivide;
    /** Optimization method. If specified, this strategy may be used by
      * providing a number of segments which are then combined or subdivided
      * in order to attain an exact number of segments.
      */
    optimize_t  optimize;
    /** If the strategy performs approximation on its own (using the registered
      * approximation technique or otherwise), this may be set to true so that
      * no further approximation is done by the toolflow.
      */
    bool        handlesApproximation;
    
    record_t(
      subdivide_t subdivide, optimize_t optimize=NULL, 
      bool handlesApproximation=false) : 
        subdivide(subdivide), optimize(optimize), 
        handlesApproximation(handlesApproximation) {

    }
    record_t() : subdivide(NULL), optimize(NULL), handlesApproximation(false) {

    }
    
    /** Entry point to be called by the tool flow.
      *
      * Proper selection of the strategy method (subdivide / optimize) is
      * done in this method as well as wrapping some code in order to minimize
      * the size of strategy implementation code.
      */
    void execute(
      LookupTable *lut, WeightsTable *weights, options_t &options) const;
  };

  #define SEGMENT_STRATEGY(id,name) extern const record_t id;
  #include "strategy-decl.h"
  #undef SEGMENT_STRATEGY
  
  /** Getter for a strategy. Returns a record_t corresponding to the ID
    * of the strategy as defined in strategy-def.h.
    */
  const record_t *get(id_t id_in); 
} // namespace segment_strategy

namespace approx_strategy {
  
  /** Type representing a single approximation strategy.
    *
    * Strategies themselves are declared in strategy-decl.h and define a 
    * symbol according to the identifier defined therein. This symbol is of
    * type record_t and contains entry points into the methods handling the
    * respective strategy.
    */
  struct record_t {
    typedef void (*handle_segment_t) (
      LookupTable *lut, WeightsTable *weights, const options_t &options, 
      const segment_t &seg, seg_data_t &y0, seg_data_t &y1);
    
    /** Strategy entry point, performing approximation of a single segment.
      */
    handle_segment_t handle_segment;

    /** Entry point to be called by the tool flow.
      *
      * Wraps around handle_segment in order to perform common tasks thus 
      * keeping the actual strategy implementation short.
      */
    void execute(
      LookupTable *lut, WeightsTable *weights, const options_t &options) const;
  };

  #define APPROX_STRATEGY(id,name) extern const record_t id;
  #include "strategy-decl.h"
  #undef APPROX_STRATEGY
  
  /** Getter for a strategy. Returns a record_t corresponding to the ID
    * of the strategy as defined in strategy-def.h.
    */
  const record_t *get(id_t id_in); 

} // namespace approx_strategy

#endif
