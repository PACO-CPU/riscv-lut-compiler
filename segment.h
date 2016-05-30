#ifndef RISCV_LUT_COMPILER_SEGMENT_H
#define RISCV_LUT_COMPILER_SEGMENT_H

/** Data type to be used for segment boundaries and values.*/
struct seg_data_t {
  enum kind_t {
    Integer,
    Double
  };  
  kind_t kind;
  union {
    double data_f;
    uint64_t data_i;
  };

  seg_data_t() : kind(Integer), data_i(0) {
  }

  seg_data_t(double v) : kind(Double), data_f(v) {
  }
  seg_data_t(uint64_t v) : kind(Integer), data_i(v) {
  }

  operator uint64_t() const {
    switch(kind) {
      case Integer: return (uint64_t)(int64_t)data_f;
      case Double:  return data_i;
    }
  }
  operator double() const {
    switch(kind) {
      case Integer: return (double)data_i;
      case Double:  return data_f;
    }
  }
};

/** Data type to use for segment interval boundaries */
typedef uint64_t seg_int_t;
/** Data type to use for segment interval values */
typedef uint64_t seg_val_t;

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
};

#endif
