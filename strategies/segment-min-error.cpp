#include "../strategies.h"
#include "../deviation.h"

/** Represents a candidate for a single optimization step.
  *
  * A candidate is a replacement option for one or two segments with one or two
  * other segments depending on the current stage being executed.
  */
struct candidate_t {
  /** index of the first segment to be replaced
    */
  ssize_t index;

  /** first segment to be inserted
    */
  segment_t segment1;

  /** second segment to be inserted
    */
  segment_t segment2;
  
  /** Error of the original situation
    */
  deviation_t error0;
  /** Error of the first segment
    */
  deviation_t error1;
  /** Error of the second segment
    */
  deviation_t error2;
};


static void _optimize_main(
  LookupTable *lut, WeightsTable *weights, 
  const options_t &options, uint32_t max_count) {

  if (lut->segments().len==max_count) return;
  
  if (lut->segments().len<1)
    assert(0 && "optimize() must be called with a non-empty set of segments");

  alp::array_t<deviation_t> errors;
  alp::array_t<segment_t> candidates;
  candidate_t new_candidate;
  candidate_t best_candidate;

  for(size_t i=0;i<lut->segments().len;i++) {
    errors.insert(lut->computeSegmentError(error_square,weights,i));
  }
  


  while(lut->segments().len>max_count) {
    best_candidate.index=-1;
    for(ssize_t i=0;i<(ssize_t)lut->segments().len-1;i++) {
      
      new_candidate.index=i;
      new_candidate.error0=errors[i]+errors[i+1];

      const segment_t &seg1=lut->segments()[i];
      const segment_t &seg2=lut->segments()[i+1];
      
      new_candidate.segment1=segment_t(
        seg1.prefix,seg2.prefix+seg2.width-seg1.prefix);
      // todo: approximate the new segment and compute its error
      
      if (
        (best_candidate.index<0) || 
        (best_candidate.error1>new_candidate.error1))
        best_candidate=new_candidate;

      
    }
    
    lut->removeSegment(best_candidate.index);
    lut->removeSegment(best_candidate.index);
    errors.remove(best_candidate.index,2);

    assert(
      lut->addSegment(best_candidate.segment1,true) &&
      "replacements of segments must never fail");

    errors.insert(best_candidate.error1,best_candidate.index);
  }

  while(lut->segments().len<max_count) {
    // todo: subdivide segments
  }

  return;
}

namespace segment_strategy {
  const record_t MIN_ERROR {
    .subdivide=NULL, // gcc (6.1.1) cannot deal with this being omitted.
    .optimize=_optimize_main
  };

};
