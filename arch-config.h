#ifndef RISCV_LUT_COMPILER_ARCH_CONFIG_H
#define RISCV_LUT_COMPILER_ARCH_CONFIG_H
#include <alpha/alpha.h>
#include <stdio.h>

struct arch_config_t {
  enum {
    Default_numSegments = 8,
    // 16 LUT records available
    Default_segmentBits = 4, 
    // 3 variables input into the PLA (select 8 segments)
    Default_selectorBits = 3,
    // 8 bits of interpolation / 256 points
    Default_interpolationBits = 8, 
  };
  // options go here.
  // todo: define more arguments depending on the LUT meta-architecture
  int numSegments;

  
  /** Number bits required to uniquely select a segment in the LUT hardware
    * core.
    *
    * A LUT hardware core generally has a power of two number of segments.
    * This power is represented in segmentBits.
    * I.e. 2^segmentBits = number of offset/slope pairs in LUT RAM
    */
  int segmentBits;

  /** Number of bits fed into the selector part of the LUT hardware core.
    *
    * Each bit is fed in together with its negated value so this value actually
    * represents *half* the number of inputs to the selector.
    */
  int selectorBits;
    
  /** Number of bits used in interpolation.
    *
    * This is the number of bits of the input word that is used to compute
    * the linear interpolation within the segment and also the number of
    * bits of offset and incline that get stored per LUT segment.
    */
  int interpolationBits;

  /** Number of interconnects between the LUT core's PLA planes.
    */
  int plaInterconnects;


  /** Threshold of internal waste of a segment at which it is preferred to
    * create a smaller threshold that renders a few elements of the domain
    * as don't cares.
    */
  double domainCutoffThreshold;

  /** Threshold of internal waste of a segment that emits a warning,
    *
    * This is especially useful when dealing with floating-point inputs that
    * can theoretically cover a large domain in a sparse manner.
    */
  double resolutionWarnThreshold;

  // todo: specify default strategies
  
  arch_config_t() : 
    numSegments(Default_numSegments),
    segmentBits(Default_segmentBits),
    selectorBits(Default_selectorBits),
    interpolationBits(Default_interpolationBits),
    plaInterconnects(Default_interpolationBits),
    domainCutoffThreshold(1), // no cutoff
    resolutionWarnThreshold(0) // no warning
    {
  }

  void parse(const char *ptr, size_t cb, const char *name);
  void parseFile(const char *fn);
  
};

#endif
