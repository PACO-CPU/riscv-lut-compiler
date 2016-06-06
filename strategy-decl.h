
#ifndef SEGMENT_STRATEGY
#define SEGMENT_STRATEGY(id,name)
#define _DEF_SEGMENT_STRATEGY
#endif

#ifndef APPROX_STRATEGY
#define APPROX_STRATEGY(id,name)
#define _DEF_APPROX_STRATEGY
#endif

/** Subdivides into equally-sized segments. */
SEGMENT_STRATEGY(UNIFORM,"uniform")

/** Subdivides into intervals reducing waste.
  *
  * Maps each individual sub-interval of the input domain into a segment
  * and combines them to reduce waste.
  * If there are too many segments left over after that, they get combined
  * so that the additional waste is minimized. If there are segments left over,
  * the largest segments get subdivided.
  */
SEGMENT_STRATEGY(BEST_FIT,"best-fit")

/** Subdivides the domain into a sequence of segments that grow exponentially
  * in size from the lower domain bound to the upper.
  *
  * This ignores discontinuities in the domain
  */
SEGMENT_STRATEGY(LOG_LEFT,"log-left")

/** Subdivides the domain into a sequence of segments that grow exponentially
  * in size from the upper domain bound to the lower.
  *
  * This ignores discontinuities in the domain
  */
SEGMENT_STRATEGY(LOG_RIGHT,"log-right")

/** Successively halves intervals, picking the one exhibiting the greatest
  * inaccuracy.
  */
SEGMENT_STRATEGY(MIN_ERROR,"min-error")

/** Successively halves intervals, picking the one yielding the greatest
  * improvement in accuracy.
  */
SEGMENT_STRATEGY(MIN_ERROR_GAIN,"min-error-gain")


/** Just interpolate the target function's values at segment boundaries.
  *
  * This yields an approximation without jumps at segment boundaries.
  */
APPROX_STRATEGY(INTERPOLATED,"interpolated")

/** Perform linear approximation of the target function minimizing absolute
  * error
  */
APPROX_STRATEGY(LINEAR,"linear")

/** Perform constant approximation of the target function minimizing quadratic
  * error.
  */
APPROX_STRATEGY(STEP,"step")

#ifdef _DEF_SEGMENT_STRATEGY
#undef SEGMENT_STRATEGY
#endif
#ifdef _DEF_APPROX_STRATEGY
#undef APPROX_STRATEGY
#endif
