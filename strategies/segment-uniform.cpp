#include "../strategies.h"

/** Computes the number of segments required to cover the entire domain
  * of the given LUT with segments of specified width.
  *
  * Optionally a subspace of the LUT's segment space can be selected.
  * Also capable of adding the respective segments to the LUT.
  *
  * Note that if segments are added they might protrude beyond the last
  * segment of this range and extend into another group of segments.
  * If this happens, an added segment covers a part of that other domain and
  * a later call to _subdivide will yield a different result
  * than it would have if no segments were added.
  * 
  * \param first First segment of the LUT's segment space to have covered.
  * \param last Final segment of the LUT's segment space to have covered.
  * \param width Width of the segments to use.
  * \param apply Set to true to create the segments and register them with lut.
  */
static uint32_t _subdivide(
  LookupTable *lut, uint32_t first, uint32_t last, uint32_t width,
  bool apply=false) {
  seg_loc_t loc;
  Bounds::interval_t interval;
  int count=0;
  /* We scan sector-by-sector to avoid the following case:
    
       +----++----+
   ____|__DD||DD__|____
       +----++----+
   
   This case could be covered with fewer segments:

         +----+
   ______|DDDD|______
         +----+

   Legend:
    '_': principal segment of don't cares, 
    'D': principal segment containing parts of the domain
    +--+
    |  |  segment
    +--+
    
  */

  for(loc.segment=first;loc.segment<=last;loc.segment++) {
    
    loc.offset=0; lut->segmentToInputSpace(loc,interval.start);
    loc.offset=1; lut->segmentToInputSpace(loc,interval.end);
    if (lut->bounds().intersectsWith(interval)) {
      if (apply) {
        if (!lut->addSegment(loc.segment,width,true)) {
          // overlap -> we have a previously created segment that reaches
          // all the way into this one. -> skip
          continue;
        }
          
      }
      count++;
      loc.segment+=width-1;
    }

  }

  return count;

}

/** Computes the minimum required segment width to cover a subrange of the
  * LUT's segment space with a given number of segments of this width.
  */
static int _min_required_width(
  LookupTable *lut, const options_t &options, 
  uint32_t first, uint32_t last, uint32_t count) {

  if (options.arch.segmentBits<options.arch.selectorBits) { 
    // binary search looking for the minimum segment width.
    uint32_t l=1,r=(1<<(options.arch.selectorBits-options.arch.segmentBits)),c;
    while(l<r) {
      c=(l+r)>>1;

      if (_subdivide(lut,first,last,c,false)<=count) {
        r=c;
      } else {
        l=c+1;
      }
        
    }
    
    return r;
    
  }
  return 1;


}

/** Subdivides a subrange of the LUT's segment space with at most max_count
  * segments, returns the number of segments used.
  */
static uint32_t _subdivide_main(
  LookupTable *lut, WeightsTable *weights, const options_t &options, 
  uint32_t first, uint32_t last, uint32_t max_count) {
  int width;
  uint32_t count;

  
  width=_min_required_width(lut,options,first,last,max_count);
  count=_subdivide(lut,first,last,width,true);

  return count;

}

namespace segment_strategy {
  const record_t UNIFORM {
    .subdivide=_subdivide_main
  };

};
