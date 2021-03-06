/** \file arch-config.h
  * \brief Structure for holding all architecture-specific configuration data
  *
  */
#ifndef RISCV_LUT_COMPILER_ARCH_CONFIG_H
#define RISCV_LUT_COMPILER_ARCH_CONFIG_H
#include <alpha/alpha.h>
#include <stdio.h>

/** Holds all architecture-specific configuration data
  *
  * Similar to (and included in) the options_t structure, the arch_config_t
  * offers a method parse and parseFile reading a file written in 
  * input format (using the first key-value section of the input lexer).
  * It interprets any key-values of the same name of fields in the arch_config_t
  * structure.
  */
struct arch_config_t {
  enum {
    Default_numSegments = 8,
    Default_wordSize = 64,
    Default_inputWords = 1,

    // 16 LUT records available
    Default_segmentBits = 4, 
    // 3 variables input into the PLA (select 8 segments)
    Default_selectorBits = 3,
    // 64 interconnects on the PLA
    Default_plaInterconnects = 64, 
    // 8 bits of interpolation / 256 points
    Default_interpolationBits = 8, 
    // 32-bit offsets in Multiply-Add unit possible
    Default_base_bits = 32, 
    // 8-bit factors for interpolationBits possible in Multiply-Add unit
    Default_incline_bits = 8, 
  };
  // options go here.
  int numSegments;
  
  /** Size of a word, in bits, in the processor. This should probably 
    * never change.
    */
  int wordSize;

  /** Number of words of input the LUT can handle internally.
    */
  int inputWords;
  
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

  /** Maximum number of bits for the base value added to the product of
    * interpolationBits and incline_bits in Multiply-Add unit.
    */
  int base_bits;

  /** Maximum number of bits for the incline value multiplied
    * interpolationBits in Multiply-Add unit.
    */
  int incline_bits;

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
    wordSize(Default_wordSize),
    inputWords(Default_inputWords),
    segmentBits(Default_segmentBits),
    selectorBits(Default_selectorBits),
    interpolationBits(Default_interpolationBits),
    plaInterconnects(Default_plaInterconnects),
    base_bits( Default_base_bits),
    incline_bits( Default_incline_bits),
    domainCutoffThreshold(1), // no cutoff
    resolutionWarnThreshold(0) // no warning
    {
  }
  
  /** Parses a buffer expected to hold key-values syntactically identical
    * to input formats.
    *
    * Available keys are the fields of this structure.
    * \param ptr Pointer into the buffer to parse.
    * \param cb Number of bytes in the buffer starting at ptr.
    * \param name Identifier of the buffer, to be used when generating
    * messages.
    * \throw SyntaxError The given buffer does not hold a valid architecture
    * config file.
    */
  void parse(const char *ptr, size_t cb, const char *name);
  /** Wrapper for the parse method accepting a file name.
    * 
    * Loads the file and executes parse with its content.
    * \throw FileIOException The file could not be opened.
    * \throw SyntaxError The file is not a valid architecture config file.
    */
  void parseFile(const char *fn);
  
};

#endif
