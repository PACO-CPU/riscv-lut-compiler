#ifndef RISCV_LUT_COMPILER_SEGMENT_H
#define RISCV_LUT_COMPILER_SEGMENT_H

#include "segdata.h"

/** Specifies a single coordinate in segment space.
  *
  * This is just a convenience data type so that only one variable needs to
  * be passed instead of two.
  */
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
  * defined over the interval [prefix,prefix+width], linearly interpolating 
  * between values y0 at prefix and y1 at prefix+width.
  */
struct segment_t {
  
  /** Prefix identifying the segment.
    *
    */
  uint32_t prefix;
  /** Width of the segment in number of atomic segment widths
    */
  uint32_t width;

  /** Value to attain at the lower bound */
  seg_data_t y0;
  /** Value to attain at the upper bound */
  seg_data_t y1;

  segment_t() { }
  segment_t(const uint32_t &prefix, const uint32_t &width) : 
    prefix(prefix), width(width) { }
  segment_t(
    const uint32_t &prefix, const uint32_t &width,
    const seg_data_t &y0, const seg_data_t &y1
    ) : prefix(prefix), width(width), y0(y0), y1(y1) { }

  bool operator==(const segment_t &b) const {
    return (b.prefix==prefix) && (b.width==width) && (b.y0==y0) && (b.y1==y1);
  }
};

#endif
