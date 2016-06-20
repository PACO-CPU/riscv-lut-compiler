#include "../strategies.h"

static void _handle_segment(
  LookupTable *lut, WeightsTable *weights, options_t &options,
  size_t index
  ) {
  
  if (lut->segments().len<=index) return;

  const segment_t &seg=lut->segments()[index];
  
  seg_data_t x_raw,y_raw,weight_raw;
  double 
    sum_wy=0,
    sum_w=0;

  uint64_t point_count=((uint64_t)seg.width)<<lut->segment_interpolation_bits();

  for(uint64_t x=0;x<point_count;x++) {
    double w=1,y;
    lut->hardwareToInputSpace(seg.prefix,x,x_raw);
    lut->evaluate(seg.prefix,x,y_raw);
    if (weights!=NULL) {
      weights->evaluate(x_raw,weight_raw);
      w=(double)weight_raw;
    }
    
    y=(double)y_raw;

    sum_wy+=w*y;
    sum_w+=w;
  }
  
  double a=sum_wy/sum_w;

  seg_data_t y( (int64_t)a );

  lut->setSegmentValues(index,y,y);

}
#define TEST_FUNC(bounds,target,code) { \
 \
  LookupTable lut(opts); \
   \
  const char *input= \
    "name=\"test\" bounds=\"" bounds "\" " \
    "segments=\"uniform\" approximation=\"linear\" " \
    "\n%%\n" \
    "target int->int\n" \
    "\n%%\n" \
    "int target(int a) { return " target "; }\n" \
    ; \
  lut.parseInput(input,strlen(input),"test lut"); \
  lut.computeSegmentSpace(); \
  lut.computePrincipalSegments(); \
  code \
}

#define TEST_SEGMENT(idx,expected_y) { \
  _handle_segment(&lut,NULL,opts,idx); \
  const segment_t &seg=lut.segments()[idx]; \
  Assertf( seg.y0==seg_data_t(expected_y), \
    "Lower segment value (%g) differs from " \
    "expected value (%g) in segment %i\n", \
    (double)seg.y0,(double)expected_y,idx); \
  Assertf( seg.y1==seg_data_t(expected_y), \
    "Upper segment value (%g) differs from " \
    "expected value (%g) in segment %i\n", \
    (double)seg.y1,(double)expected_y,idx); \
}

unittest( 
  /* 
    testing:
      _handle_segment
  */

  options_t opts;
  opts.arch.selectorBits=4;
  opts.arch.segmentBits=3;
  opts.arch.interpolationBits=8;

  TEST_FUNC( "(0,1023)", "a",
    TEST_SEGMENT(0, 31);
    TEST_SEGMENT(1, 95);
  )

)
#undef TEST_FUNC
#undef TEST_SEGMENT


static void _execute(
  LookupTable *lut, WeightsTable *weights, options_t &options) {
  
  for(size_t idx=0;idx<lut->segments().len;idx++)
    _handle_segment(lut,weights,options,idx);
}

namespace approx_strategy {
  const record_t STEP {
    .execute=_execute
  };

};
