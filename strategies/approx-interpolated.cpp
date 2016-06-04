#include "../strategies.h"

static void _execute(
  LookupTable *lut, WeightsTable *weights, options_t &options) {


}

namespace approx_strategy {
  const record_t INTERPOLATED {
    .execute=_execute
  };

};
