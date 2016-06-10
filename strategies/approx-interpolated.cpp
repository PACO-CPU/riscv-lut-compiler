#include "../strategies.h"

static void _execute(
  LookupTable *lut, WeightsTable *weights, options_t &options) {
  
  const alp::array_t<segment_t> &segments=lut->segments();
  for(size_t i=0;i<segments.len;i++) {
    lut->setSegmentValues(
      i,
      lut->evaluate(seg_loc_t(segments[i].x0,0)),
      lut->evaluate(seg_loc_t(segments[i].x1,1)));
  }

}

namespace approx_strategy {
  const record_t INTERPOLATED {
    .execute=_execute
  };

};
