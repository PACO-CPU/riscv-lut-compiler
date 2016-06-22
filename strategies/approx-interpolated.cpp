#include "../strategies.h"

static void _handle_segment(
  LookupTable *lut, WeightsTable *weights, options_t &options,
  size_t index
  ) {

  lut->setSegmentValues(
    index,
    lut->evaluate(index,0),
    lut->evaluate(
      index,
      (lut->segments()[index].width<<lut->segment_interpolation_bits())));
}

namespace approx_strategy {
  const record_t INTERPOLATED {
    .handle_segment=_handle_segment
  };

};
