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
  
  /** Set to true to indicate the two segments to be united are continuous */
  bool continuous;
  /** Beginning of the interval that was removed, used for zeroing weights */
  seg_data_t remove_start;
  /** End of the interval that was removed, used for zeroing weights */
  seg_data_t remove_end;
};


static void _optimize(
  LookupTable *lut, WeightsTable *weights, 
  const options_t &options, uint32_t max_count,
  bool use_gain) {

  if (lut->segments().len==max_count) return;
  
  if (lut->segments().len<1)
    assert(0 && "optimize() must be called with a non-empty set of segments");
  
  // maps segment indices in the LUT to errors with respect to the target func
  alp::array_t<deviation_t> errors;
  
  // fetch our approximation strategy (need it to compute errors)
  if (lut->approximation_strategy()==approx_strategy::INVALID)
    throw RuntimeError(
      "cannot perform error minimization without an approximation strategy\n");
  const approx_strategy::record_t *approximation=
    approx_strategy::get(lut->approximation_strategy());

  // having a weights table (syntactically) is very handy as we use it
  // to zero out values covered by intervals that were don't cares before.
  if (weights==NULL) {
    weights=new WeightsTable(lut->bounds());
  }

  // compute errors for the segments to be optimized. For that we need to
  // perform approximation first
  approximation->execute(lut,weights,options);
  for(size_t i=0;i<lut->segments().len;i++) {
    errors.insert(lut->computeSegmentError(error_square,weights,i));
  }
  
  // as we cannot return weights we need to delete whatever we crated here,
  // so just grab it here and we can drop it later without problems
  weights->grab();
  
  candidate_t new_candidate;
  candidate_t best_candidate;

  // combine segments until we reach max_count (from above)
  while(lut->segments().len>max_count) {
    best_candidate.index=-1;
    for(ssize_t i=0;i<(ssize_t)lut->segments().len-1;i++) {
      
      new_candidate.index=i;
      new_candidate.error0=errors[i]+errors[i+1];

      const segment_t &seg1=lut->segments()[i];
      const segment_t &seg2=lut->segments()[i+1];
      
      new_candidate.segment1=segment_t(
        seg1.prefix,seg2.prefix+seg2.width-seg1.prefix);

      new_candidate.segment2=segment_t(
        seg1.prefix+seg1.width,seg2.prefix-seg1.prefix-seg1.width);
      lut->hardwareToInputSpace(
        new_candidate.segment2,0,new_candidate.remove_start);
      lut->hardwareToInputSpace(
        new_candidate.segment2,(uint64_t)-1,new_candidate.remove_end);
      
      WeightsTable *curWeights=weights;
      if (seg1.prefix+seg1.width<seg2.prefix) {
        curWeights=new WeightsTable(weights);
        curWeights->grab();
        curWeights->setZeroRange(
          new_candidate.remove_start,new_candidate.remove_end);
        new_candidate.continuous=false;
        
      } else {
        curWeights=weights;
        curWeights->grab();
        new_candidate.continuous=true;
      }

      approximation->handle_segment(
        lut,curWeights,options,new_candidate.segment1,
        new_candidate.segment1.y0,new_candidate.segment1.y1);
      new_candidate.error1=lut->computeSegmentError(
        error_square,curWeights,new_candidate.segment1);

      curWeights->drop();

      if (new_candidate.error1>new_candidate.error0) continue;
      
      bool better=best_candidate.index<0;
      
      if (use_gain) {
        better|= 
          (best_candidate.error0.mean/best_candidate.error1.mean) <
          (new_candidate.error0.mean/new_candidate.error1.mean);
      } else {
        better|=
          (best_candidate.error1>new_candidate.error1);
      }
      if (better) 
        best_candidate=new_candidate;
      
    }
    
    lut->removeSegment(best_candidate.index);
    lut->removeSegment(best_candidate.index);
    errors.remove(best_candidate.index,2);

    assert(
      lut->addSegment(best_candidate.segment1,true) &&
      "replacements of segments must never fail");

    errors.insert(best_candidate.error1,best_candidate.index);
    if (!best_candidate.continuous)
      weights->setZeroRange(
        best_candidate.remove_start,
        best_candidate.remove_end);
  }
  

  // subdivide segments until we reach max_count (from below)
  while(lut->segments().len<max_count) {
    best_candidate.index=-1;
    for(ssize_t i=0;i<(ssize_t)lut->segments().len;i++) {
      
      const segment_t &seg0=lut->segments()[i];
      if (seg0.width<2) continue;
      
      new_candidate.index=i;
      new_candidate.error0=errors[i];

      new_candidate.segment1=seg0;
      new_candidate.segment2=seg0;

      for(uint32_t width1=1;width1<seg0.width;width1++) {
        new_candidate.segment1.width=width1;
        new_candidate.segment2.prefix=new_candidate.segment1.prefix+width1;
        new_candidate.segment2.width=seg0.width-width1;

        approximation->handle_segment(
          lut,weights,options,new_candidate.segment1,
          new_candidate.segment1.y0,new_candidate.segment1.y1);
        approximation->handle_segment(
          lut,weights,options,new_candidate.segment2,
          new_candidate.segment2.y0,new_candidate.segment2.y1);

        new_candidate.error1=
          lut->computeSegmentError(
            error_square,weights,new_candidate.segment1);
        new_candidate.error2=
          lut->computeSegmentError(
            error_square,weights,new_candidate.segment2);


        if (new_candidate.error1+new_candidate.error2>new_candidate.error0) 
          continue;
        
        bool better=best_candidate.index<0;
        
        if (use_gain) {
          better|= 
            (
              best_candidate.error0.mean/
              (best_candidate.error1.mean+best_candidate.error2.mean)) <
            (
              new_candidate.error0.mean/
              (new_candidate.error1.mean+new_candidate.error2.mean));
        } else {
          better|=
            (
              (best_candidate.error1+best_candidate.error1)>
              (new_candidate.error1+new_candidate.error2));
        }
        if (better) 
          best_candidate=new_candidate;
      }
    }

    if (best_candidate.index<0) {
      // cannot subdivide: all segments are of minimal width
      // fixme: emit an info message here?
      break;
    }
    lut->removeSegment(best_candidate.index);
    errors.remove(best_candidate.index);

    assert(
      lut->addSegment(best_candidate.segment1,true) &&
      "replacements of segments must never fail");
    assert(
      lut->addSegment(best_candidate.segment2,true) &&
      "replacements of segments must never fail");

    errors.insert(best_candidate.error1,best_candidate.index);
    errors.insert(best_candidate.error2,best_candidate.index);

  }

  weights->drop();
  return;
}

static void _optimize_normal(
  LookupTable *lut, WeightsTable *weights, 
  const options_t &options, uint32_t max_count) {
  _optimize(lut,weights,options,max_count,false);
}
static void _optimize_gain(
  LookupTable *lut, WeightsTable *weights, 
  const options_t &options, uint32_t max_count) {
  _optimize(lut,weights,options,max_count,true);
}
namespace segment_strategy {
  const record_t MIN_ERROR {
    .subdivide=NULL, // gcc (6.1.1) cannot deal with this being omitted.
    .optimize=_optimize_normal
  };
  const record_t MIN_ERROR_GAIN {
    .subdivide=NULL, // gcc (6.1.1) cannot deal with this being omitted.
    .optimize=_optimize_gain
  };

};
