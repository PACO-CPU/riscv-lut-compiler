#include "../strategies.h"

static void _execute(
  LookupTable *lut, WeightsTable *weights, options_t &options) {
  
  // todo: implement

}

namespace approx_strategy {
  const record_t MIN_ERROR {
    .execute=_execute
  };

};
