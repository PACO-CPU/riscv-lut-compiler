#ifndef RISCV_LUT_COMPILER_SEGMENT_H
#define RISCV_LUT_COMPILER_SEGMENT_H

#include "segdata.h"

struct seg_loc_t {
  uint32_t segment;
  double offset;

  seg_loc_t() : segment(0), offset(0) { }
  seg_loc_t(uint32_t segment) : segment(segment), offset(0) { }
  seg_loc_t(uint32_t segment, double offset) : 
    segment(segment), offset(offset) { 
  }
  
  bool operator<=(const seg_loc_t &v) const {
    if (segment<v.segment) return true;
    if (segment>v.segment) return false;
    return offset<=v.offset;
  }
  
  bool operator>=(const seg_loc_t &v) const { return v<=*this; } 
  bool operator<(const seg_loc_t &v) const { return !(v<=*this); } 
  bool operator>(const seg_loc_t &v) const { return !(*this<=v); } 

  bool operator==(const seg_loc_t &v) const {
    return (segment==v.segment) && (offset==v.offset);
  }
  
};

/** Structure representing a single LUT segment.
  *
  * Mathematically this is represents one part of a partwise linear function
  * defined over the interval [x0,x1], linearly interpolating between
  * values y0 at x0 and y1 at x2.
  */
struct segment_t {
  /** Lower bound of the segment's interval in segment space.
    *
    * This is the index of a segment whose lower bounds make up the 
    * lower bounds of this segment.
    */
  uint32_t x0;
  /** Upper bound of the segment's interval inn segment space.
    *
    * This is the index of a segment whose upper bounds make up the 
    * upper bounds of this segment.
    */
  uint32_t x1;

  /** Value to attain at the lower bound */
  seg_data_t y0;
  /** Value to attain at the upper bound */
  seg_data_t y1;

  segment_t() { }
  segment_t(const uint32_t &x0, const uint32_t &x1) : x0(x0), x1(x1) { }
  segment_t(
    const uint32_t &x0, const uint32_t &x1,
    const seg_data_t &y0, const seg_data_t &y1
    ) : x0(x0), x1(x1), y0(y0), y1(y1) { }
};

#endif
