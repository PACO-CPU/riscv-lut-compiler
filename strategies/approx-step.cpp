#include "../strategies.h"

static void _handle_segment(
  LookupTable *lut, WeightsTable *weights, const options_t &options,
  const segment_t &seg, seg_data_t &y0, seg_data_t &y1
  ) {
  
  seg_data_t x_raw,y_raw,weight_raw;
  double 
    sum_wy=0,
    sum_w=0;

  uint64_t point_count=((uint64_t)seg.width)<<lut->segment_interpolation_bits();

  for(uint64_t x=0;x<point_count;x++) {
    double w=1,y;
    lut->hardwareToInputSpace(seg,x,x_raw);
    lut->evaluate(seg,x,y_raw);
    if (weights!=NULL) {
      weights->evaluate(x_raw,weight_raw);
      w=(double)weight_raw;
    }
    
    y=(double)y_raw;

    sum_wy+=w*y;
    sum_w+=w;
  }
  
  double a=sum_wy/sum_w;
  
  y0=(int64_t)a;
  y1=y0;

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
  seg_data_t y0,y1; \
  _handle_segment(&lut,NULL,opts,lut.segments()[idx],y0,y1); \
  Assertf( y0==seg_data_t(expected_y), \
    "Lower segment value (%g) differs from " \
    "expected value (%g) in segment %i\n", \
    (double)y0,(double)expected_y,idx); \
  Assertf( y1==seg_data_t(expected_y), \
    "Upper segment value (%g) differs from " \
    "expected value (%g) in segment %i\n", \
    (double)y1,(double)expected_y,idx); \
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

namespace approx_strategy {
  const record_t STEP {
    .handle_segment=_handle_segment
  };

};
