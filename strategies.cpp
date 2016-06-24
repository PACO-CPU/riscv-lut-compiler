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

  void record_t::execute(
    LookupTable *lut, WeightsTable *weights, options_t &options) const {
    if (lut->segments().len>0) { // secondary strategy
      
      if (optimize!=NULL) {
        optimize(lut,weights,options,1uL<<options.arch.selectorBits);
        return;
      } else if (subdivide==NULL) 
        throw RuntimeError(
          "this segmentation strategy cannot be used as "
          "secondary segmentation strategy");

      alp::array_t<segment_t> primary=lut->segments().dup();
      uint32_t n_total=(1<<options.arch.segmentBits);
      if (primary.len>=n_total) {
        alp::logf(
          "WARNING: secondary segmentation has no segments left\n",
          alp::LOGT_WARNING);
        return;
      }

      uint32_t n_avail=n_total-primary.len;

      lut->clearSegments();

      for(size_t i=0;i<primary.len;i++) {
        
        uint32_t n_each=n_avail/(primary.len-i);
        // number of segments distributed evenly
        // + 1 if there are still remainders
        // + 1 (original segment)
        uint32_t n_this=n_each + (((n_avail-n_each*(primary.len-i))>0) ? 1 : 0) +1;

        segment_t &seg=primary[i];
        
        // philosophy of secondary segmentation is a per-segment re-segmentation.
        // therefore we perform a local computation of width which allows 
        // different primary segments to be subdivided with different widths thus
        // the resulting segmentation may not be uniform anymore.
        uint32_t count=subdivide(
          lut,weights,options,seg.prefix,seg.prefix+seg.width,n_this);
        
        // we freed one segment (the original one) and consumed count ones.
        n_avail=n_avail+1-count;

      }

    } else { // primary strategy 
      if (optimize!=NULL) {
        lut->computePrincipalSegments();
        optimize(lut,weights,options,lut->num_primary_segments());

      } else if (subdivide!=NULL) {
        subdivide(lut,weights,options,0,(1<<options.arch.segmentBits)-1,
          lut->num_primary_segments());
      } else 
        assert(0 && "segmentation strategy not implemented");
    }

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

  void record_t::execute(
    LookupTable *lut, WeightsTable *weights, options_t &options) const {

    seg_data_t y0,y1;

    for(size_t idx=0;idx<lut->segments().len;idx++) {
      handle_segment(lut,weights,options,lut->segments()[idx],y0,y1);
      lut->setSegmentValues(idx,y0,y1);
    }
  }

} // namespace approx_strategy

