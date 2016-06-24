#include "../strategies.h"

static void _handle_segment(
  LookupTable *lut, WeightsTable *weights, const options_t &options,
  const segment_t &seg, seg_data_t &y0, seg_data_t &y1
  ) {
  
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
    lut->hardwareToInputSpace(seg,x,x_raw);
    lut->evaluate(seg,x,y_raw);
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
  
  if (!(sum_w>0)) {
    y0=(int64_t)0;
    y1=y0;
  } else if (point_count<2) {
    y0=(int64_t) (sum_wy/sum_w);
    y1=y0;
  } else {
    double a=
      (sum_wxy - sum_wy*sum_wx/sum_w) /
      (sum_wxx - sum_wx*sum_wx/sum_w);
    double b=
      (sum_wy-a*sum_wx) / 
      (sum_w);
    
    y0=(int64_t)b;
    y1=(int64_t)(b+a*point_count-1);
  }

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
  seg_data_t y0,y1; \
  _handle_segment(&lut,NULL,opts,lut.segments()[idx],y0,y1); \
  Assertf( y0==seg_data_t(expected_y0), \
    "Lower segment value (%g) differs from " \
    "expected value (%g) in segment %i\n", \
    (double)y0,(double)expected_y0,idx); \
  Assertf( y1==seg_data_t(expected_y1), \
    "Upper segment value (%g) differs from " \
    "expected value (%g) in segment %i\n", \
    (double)y1,(double)expected_y1,idx); \
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

namespace approx_strategy {
  const record_t LINEAR {
    .handle_segment=_handle_segment
  };

};
