#include "../strategies.h"

enum {

  /** Logarithmic subdivision strategy doubling interval size with each step.
    *
    * In addition to these intervals another one may be inserted if the interval
    * width is not representable as a number 2^k-1 for any natural number k.
    * The sequence of intervals created is always sorted by width.
    */
  METHOD_REMAINDER=0,
  /** Logarithmic subdivision strategy using the binary representation of the
    * interval to be subdivided.
    */
  METHOD_BINARY=1,

  METHOD_DEFAULT=METHOD_REMAINDER,
};

static void _compute_widths(
  LookupTable *lut, const options_t &options,
  uint32_t width, uint32_t max_count, alp::sorted_array_t<uint32_t> &res) {
  
  int method=METHOD_DEFAULT;
  KeyValue *kv;

  kv=lut->getKeyValue("log-method");
  if (kv!=NULL) {
    const alp::string &ids=kv->val_str();
    if (ids=="binary") method=METHOD_BINARY;
    else if (ids=="remainder") method=METHOD_REMAINDER;
    else throw RuntimeError(
      alp::string::Format("unknown log subdivision method: '%s'",ids.ptr));
  }

  switch(method) {
    
    case METHOD_BINARY: {
      for(uint32_t i=0;i<32;i++) if (width&(1uL<<i)) res.insert(1ul<<i);
      break;
    }

    case METHOD_REMAINDER: {
      uint32_t w=1, remain=width;
      while(w<=remain) { 
        res.insert(w);
        remain-=w;
        w<<=1;
      }
      if (remain>0) res.insert(remain);
      break;
    }

    default:
      assert(0 && "Unimplemented log subdivision method");
  }

  if (res.len>max_count) {
    uint32_t w=0;
    while(res.len>=max_count) {
      w+=res.ptr[0];
      res.remove(0);
    }
    res.insert(w);
  }
  
}


static uint32_t _subdivide(
  LookupTable *lut, const options_t &options, uint32_t first, uint32_t last,
  uint32_t max_count, bool reverse) {

  if (last<first) return 0;
  
  alp::sorted_array_t<uint32_t> widths;
  uint32_t offset=first;
  _compute_widths(lut,options,last-first+1,max_count,widths);
  
  if (widths.len<1) return 0;

  printf("widths: ");
  for(size_t i=0;i<widths.len;i++) printf("%i ",widths[i]);
  printf("\n");

  #define ADD_SEGMENT { \
    assert( \
      lut->addSegment(offset,widths[i],true) && \
      "segment overlap must not occur here"); \
    offset+=widths[i]; \
  }

  if (reverse) { 
    for(ssize_t i=(ssize_t)widths.len-1;i>=0;i--) ADD_SEGMENT
  } else {
    for(ssize_t i=0;i<=(ssize_t)widths.len-1;i++) ADD_SEGMENT
  }

  #undef ADD_SEGMENT

  return widths.len;
}

static uint32_t _subdivide_left(
  LookupTable *lut, WeightsTable *weights, 
  const options_t &options, uint32_t first, uint32_t last,
  uint32_t max_count) {
  return _subdivide(lut,options,first,last,max_count,false);
}
static uint32_t _subdivide_right(
  LookupTable *lut, WeightsTable *weights, 
  const options_t &options, uint32_t first, uint32_t last,
  uint32_t max_count) {
  return _subdivide(lut,options,first,last,max_count,true);
}


namespace segment_strategy {
  const record_t LOG_LEFT {
    .subdivide=_subdivide_left
  };
  const record_t LOG_RIGHT {
    .subdivide=_subdivide_right
  };

};
