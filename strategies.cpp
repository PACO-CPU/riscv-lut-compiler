#include "strategies.h"

namespace segment_strategy {
  const record_t *get(id_t id_in) {
    if (0) {
    #define SEGMENT_STRATEGY(id,name) } else if (id_in==ID_##id) { return &id;
    #include "strategy-decl.h"
    #undef SEGMENT_STRATEGY
    } else
      assert(0 && "Invalid segmentation strategy ID propagated");
    return NULL;
  }
} // namespace segment_strategy

namespace approx_strategy {
  const record_t *get(id_t id_in) {
    if (0) {
    #define APPROX_STRATEGY(id,name) } else if (id_in==ID_##id) { return &id;
    #include "strategy-decl.h"
    #undef APPROX_STRATEGY
    } else
      assert(0 && "Invalid approximation strategy ID propagated");
    return NULL;
  }

} // namespace approx_strategy

