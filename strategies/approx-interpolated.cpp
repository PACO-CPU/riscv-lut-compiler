#include "../strategies.h"

static void _handle_segment(
  LookupTable *lut, WeightsTable *weights, const options_t &options,
  const segment_t &seg, seg_data_t &y0, seg_data_t &y1
  ) {
  
  y0=lut->evaluate(seg,0);
  y1=lut->evaluate(seg,(seg.width<<lut->segment_interpolation_bits()));
}

namespace approx_strategy {
  const record_t INTERPOLATED {
    .handle_segment=_handle_segment
  };

};
