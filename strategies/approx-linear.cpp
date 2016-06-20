#include "../strategies.h"

static void _handle_segment(
  LookupTable *lut, WeightsTable *weights, options_t &options,
  size_t index
  ) {
  
  if (lut->segments().len<=index) return;

  const segment_t &seg=lut->segments()[index];
  
  seg_data_t x_raw,y_raw,weight_raw;
  double 
    sum_wx=0,
    sum_wy=0,
    sum_wxy=0,
    sum_w=0,
    sum_wxx=0;

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
    sum_wx+=w*x;
    sum_wxy+=w*x*y;
    sum_w+=w;
    sum_wxx+=w*x*x;
  }
  
  double a=
    (sum_wxy - sum_wy*sum_wx/sum_w) /
    (sum_wxx - sum_wx*sum_wx/sum_w);
  double b=
    (sum_wy-a*sum_wx) / 
    (sum_w);

  seg_data_t y0( (int64_t)b ), y1( (int64_t)(b+a*point_count-1) );

  lut->setSegmentValues(index,y0,y1);

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

#define TEST_SEGMENT(idx,expected_y0,expected_y1) { \
  _handle_segment(&lut,NULL,opts,idx); \
  const segment_t &seg=lut.segments()[idx]; \
  Assertf( seg.y0==seg_data_t(expected_y0), \
    "Lower segment value (%g) differs from " \
    "expected value (%g) in segment %i\n", \
    (double)seg.y0,(double)expected_y0,idx); \
  Assertf( seg.y1==seg_data_t(expected_y1), \
    "Upper segment value (%g) differs from " \
    "expected value (%g) in segment %i\n", \
    (double)seg.y1,(double)expected_y1,idx); \
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
    TEST_SEGMENT(0, 0,63);
    TEST_SEGMENT(1, 64,127);
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
  const record_t LINEAR {
    .execute=_execute
  };

};
