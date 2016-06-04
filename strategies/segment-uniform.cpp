#include "../strategies.h"

static void _execute(
  LookupTable *lut, WeightsTable *weights, options_t &options) {


}

namespace segment_strategy {
  const record_t UNIFORM {
    .execute=_execute
  };

};
