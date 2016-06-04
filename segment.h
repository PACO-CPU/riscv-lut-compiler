#ifndef RISCV_LUT_COMPILER_SEGMENT_H
#define RISCV_LUT_COMPILER_SEGMENT_H

#include "segdata.h"



/** Structure representing a single LUT segment.
  *
  * Mathematically this is represents one part of a partwise linear function
  * defined over the interval [x0,x1], linearly interpolating between
  * values y0 at x0 and y1 at x2.
  */
struct segment_t {
  /** Lower bound of the segment's interval */
  seg_data_t x0;
  /** Upper bound of the segment's interval */
  seg_data_t x1;

  /** Value to attain at the lower bound */
  seg_data_t y0;
  /** Value to attain at the upper bound */
  seg_data_t y1;

  segment_t() { }
  segment_t(const seg_data_t &x0, const seg_data_t &x1) : x0(x0), x1(x1) { }
  segment_t(
    const seg_data_t &x0, const seg_data_t &x1,
    const seg_data_t &y0, const seg_data_t &y1
    ) : x0(x0), x1(x1), y0(y0), y1(y1) { }
};

#endif
