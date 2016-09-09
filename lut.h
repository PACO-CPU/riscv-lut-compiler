#ifndef RISCV_LUT_COMPULER_LUT_H
#define RISCV_LUT_COMPULER_LUT_H

#include "error.h"
#include "keyvalue.h"
#include "segment.h"
#include "target-types.h"
#include "util.h"
#include "dlib.h"
#include "bounds.h"
#include "arch-config.h"
#include "options.h"
#include "strategy-def.h"
#include "weights.h"
#include "deviation.h"
#include "qmc.h"

#include <alpha/alpha.h>


/** represents a single Lookup table.
  *
  * Contains all the information obtained from Input or intermediate files
  * and generated afterwards.
  *
  * All keyvalues read from input files are stored but meaningful ones are
  * interpreted as well and stored in appropriate fields.
  *
  * During translation from input format to segments, a new input domain space
  * is defined, the *segment space*. This is a power of two interval in which
  * the target function is to be approximated. Inputs outside of this interval
  * are considered don't cares, thus the approximation is extended periodically
  * beyond the segment space boundaries.
  * Segment space is further partitioned into a number of segments defined by
  * the number of bits arch_config_t::selectorBits fed into the LUT's address
  * translation PLA. The output of this PLA consists of 
  * arch_config_t::segmentBits bits, which in turn limits the number of 
  * segments to be used from this space.
  *
  */
class LookupTable {
  public:
    typedef void (*target_func_t)(seg_data_t *res, const seg_data_t *arg0);
   
  protected:
  

    // process-specific options
    alp::string _cmdCompileSO;
    
    
    /** Lookup table identifier generated *externally* and guaranteed to be 
      * unique among all Lookup tables used in a single program.
      */
    alp::string _ident;

    arch_config_t _arch;

    // keyvalues overridden by input files (architecture defaults)
    int _num_segments;
    int _num_primary_segments;
    segment_strategy::id_t _strategy1;
    segment_strategy::id_t _strategy2;
    Bounds _explicit_segments;
    approx_strategy::id_t _approximation_strategy;
    alp::string _fn_weights;
    Bounds      _bounds;


    
    // fields loaded/generated from input files only:
    alp::array_t<KeyValue*> _keyvalues;

    alp::string _target_name;
    target_type_t _target_result_type;
    alp::array_t<target_type_t> _target_argument_types;

    alp::string _c_code;
    
    dynamic_library_t *_target_lib;
    target_func_t _target_func;
    
    // segments (loaded from intermediate or generated from input)
    seg_data_t _segment_space_offset;
    int _segment_space_width;

    alp::array_t<segment_t> _segments;

    // generated LUT configuration held in arrays
    char* and_plane_conf;
    char* or_plane_conf;
    char* connection_plane_conf;
    seg_data_t* factor_output;
    seg_data_t* offset_output;
    bool use_multiply_add;
    
    /** LUT configuration bitstream
      */
    alp::array_t<uint64_t> _config_words;

  public:
    /** Constructor.
      *
      * Creates an emtpy Lookup table.
      */
    LookupTable();
    /** Constructor.
      *
      * Uses architecture-specific initialization
      */
    LookupTable(const arch_config_t &cfg);
    /** Constructor.
      *
      * Uses architecture-specific and program flow-specific initialization
      */
    LookupTable(const options_t &opts);

    ~LookupTable();
    
    /** Getter for this LUT's identifier */
    const alp::string &ident() const { return _ident; }
    
    int num_segments() const { return _num_segments; }
    int num_primary_segments() const { return _num_primary_segments; }
    segment_strategy::id_t strategy1() const { return _strategy1; }
    segment_strategy::id_t strategy2() const { return _strategy2; }
    const Bounds &explicit_segments() const { return _explicit_segments; }
    approx_strategy::id_t approximation_strategy() const { 
      return _approximation_strategy; 
    }
    /** Returns the file name of a weights file to be used in conjunction
      * with this lut if specified by a key-value during input parsing.
      */
    const alp::string &fn_weights() const { return _fn_weights; }

    /** Returns a constant view into our domain as represented by our
      * bounds instance.*/
    const Bounds &bounds() { return _bounds; }
    
    /** Returns the segment space offset, being the first value to be 
      * covered by our domain.
      */
    const seg_data_t &segment_space_offset() const { 
      return _segment_space_offset; 
    }
    /** Returns the width of the segment space as the exponent of that 
      * power-of-two value.
      */
    int segment_space_width() const { return _segment_space_width; }
    
    /** Returns the number of bits fed into the interpolation logic.
      *
      * Although this number is specified by arch_config_t::interpolationBits,
      * it may actually be lower than that if the domain is so small that there
      * are no more bits.
      */
    int segment_interpolation_bits() const {
      int res=_segment_space_width-_arch.selectorBits;
      if (res>_arch.interpolationBits) {
        res=_arch.interpolationBits;
      }
      return res;
    }

    /** Attempts to retrieve a named key-value.
      *
      * \param key Name of the keyvalue to retrieve.
      * \return index of the keyvalue or -1 if it was not found.
      */
    ssize_t findKeyValue(const alp::string &key) const;
    /** Attempts to retrieve a named key-value.
      *
      * \param key Name of the keyvalue to retrieve.
      * \return Pointer to the keyvalue if it exists, NULL otherwise.
      */
    KeyValue *getKeyValue(const alp::string &key) const;
    
    
    
    /** Parses a string into an segmentation strategy.
      *
      * Basically just a lookup of strings
      */
    static segment_strategy::id_t ParseSegmentStrategy(const alp::string &s);

    /** Parses a string into an segmentation strategy.
      *
      * Basically just a lookup of strings
      */
    static approx_strategy::id_t ParseApproxStrategy(const alp::string &s);
    
    /** Parses an input format buffer and integrates its content into this
      * instance.
      * 
      * Any additional information that was read from previous calls to
      * parseInput, parseIntermediate or was otherwise generated will be lost.
      *
      * \param ptr Pointer into the beginning of the buffer to read from.
      * \param cb Length of the buffer starting at ptr, in bytes.
      * \param name Name used to identify the compilation unit in debug/error
      * output
      */
    void parseInput(const char *ptr, size_t cb, const char *name=NULL);

    /** Parses an input format file using parseInput.
      *
      * \param fn File name of the file to be read
      * \throw FileIOException The given file could not be read.
      */
    void parseInputFile(const char *fn);
    
    
    /** Parses a buffer expected to be in intermediate representation.
      * 
      * Any additional information that was read from previous calls to
      * parseInput, parseIntermediate or was otherwise generated will be lost.
      *
      * \param ptr Pointer into the beginning of the buffer to read from.
      * \param cb Length of the buffer starting at ptr, in bytes.
      */
    void parseIntermediate(const char *ptr, size_t cb, const char *name=NULL);

    /** Parses an intermediate format file using parseIntermediate.
      *
      * \param fn File name of the file to be read
      * \throw FileIOException The given file could not be read.
      * \param name Name used to identify the compilation unit in debug/error
      * output
      */
    void parseIntermediateFile(const char *fn);
    
    /** Generates a representation of the Lookup table in intermediate
      * format.
      *
      * The result is a null-terminated string in intermediate format.
      *
      * \param res String variable receiving the generated code.
      */
    void generateIntermediateFormat(alp::string &res);
    /** Writes a representation of this Lookup table to an intermediate file.
      *
      * \param fn File name to write to
      * \throw FileIOException The file could not be written to.
      * \see generateIntermediateFormat
      */
    void saveIntermediateFile(const char *fn);
    
    
    /** Generates a representation of the Lookup table in final output format.
      *
      * Outputs C code defining a constant buffer of our configuration bits.
      * \param res String variable receiving the generated code.
      */
    void generateOutputFormat(alp::string &res);

    /** Saves the final output code output to a file.
      *
      * \param fn File name to write to
      * \throw FileIOException The file could not be written to.
      */
    void saveOutputFile(const char *fn);

    void generateOutputDumpFormat(alp::string &res);
    void saveOutputDumpFile(const char *fn);
    
    
    /** Computes the segment space of this LUT.
      *
      * The segment space is an interval of possible input values (the domain)
      * which is addressable by LUT segments.
      * This has always a power of two extent and dictates what bits of an
      * input word are used in the LUT core's PLA component.
      * After the segment space was computed its segments can be addressed
      * via the segment space evaluation method specifying the segment's 
      * identifying bits and an offset into this segment.
      *
      * Each segment in segment space is indexed by a number of bits defined
      * in arch_config_t::selectorBits.
      *
      * \see evaluate
      */
    void computeSegmentSpace();
    
    /** Creates a new list of segments selecting all segments of width 1 that
      * intersect the input domain.
      */
    void computePrincipalSegments();
    
    /** Convertes a coordinate from segment space to input space.
      *
      * \param segment Index of the segment to address. See computeSegmentSpace
      * for more information.
      * \param offset Offset into the indexed segment. See computeSegmentSpace
      * for more information.
      * \param inp Result of the coordinate translation.
      */
    void segmentToInputSpace(const seg_loc_t &seg, seg_data_t &inp);
    /** Convertes a coordinate from segment space to input space.
      *
      * \param segment Index of the addressed segment. See computeSegmentSpace
      * for more information.
      * \param offset Offset into the indexed segment. See computeSegmentSpace
      * for more information.
      * \param inp Location to translate to segment space
      */
    void inputToSegmentSpace(seg_loc_t &seg, const seg_data_t &inp);

    /** Convertes a coordinate from hardware space to input space.
      *
      * \param seg Segment of the hardware coordinates.
      * \param offset Interpolation bits.
      * \param inp Result of the coordinate translation.
      */
    void hardwareToInputSpace(
      const segment_t &seg, uint64_t offset, seg_data_t &inp);
    /** Convertes a coordinate from hardware space to input space.
      *
      * \param addr Index of the segment.
      * \param offset Interpolation bits.
      * \param inp Result of the coordinate translation.
      */
    void hardwareToInputSpace(size_t addr, uint64_t offset, seg_data_t &inp);
    
    /** Returns a constant view into the segments registered.
      */
    const alp::array_t<segment_t> &segments() const { return _segments; }
    /** Adds a new segment into the LUT.
      *
      * \param seg Segment to insert.
      * \param failOnOverlap Set to true to just return false if the new
      * segment is not disjoint with existing ones. Otherwise an exception is
      * thrown.
      * \throw RuntimeError The given segment overlaps with another segment
      * and failOnOverlap was set to false.
      * \return true iff the segment was successfully added
      */
    bool addSegment(const segment_t &seg, bool failOnOverlap);    
    
    /** Adds a new segment into the LUT.
      *
      * Translates the input arguments into segment space and adds a new
      * constant 0 segment with those boundaries.
      * \see addSegment for further information.
      *
      * \param x0 Lower boundary of the segment in input space.
      * \param x1 Upper boundary of the segment in input space.
      * \param failOnOverlap Set to true to just return false if the new
      * segment is not disjoint with existing ones. Otherwise an exception is
      * thrown.
      * \throw RuntimeError The given segment overlaps with another segment
      * and failOnOverlap was set to false.
      * \return true iff the segment was successfully added
      */
    bool addSegment(
      const seg_data_t &x0, const seg_data_t &x1, bool failOnOverlap);

    /** Version of addSegment accepting proper segment boundaries
      *
      * The segment boundaries are clamped to valid values within the 
      * segment space before adding it.
      * Aborts returning false if it ends up empty.
      */
    bool addSegment(uint32_t prefix, uint32_t width, bool failOnOverlap);


    void removeSegment(size_t index) {
      _segments.remove(index);
    }
    void clearSegments() {
      _segments.clear();
    }
    /** Alters an indexed segment by setting the values it should attain at
      * the lower and upper boundaries.
      *
      * \param index Index of the segment.
      * \param y0 Value to attain at the lower boundary.
      * \param y1 Value to attain at the upper boundary.
      */
    void setSegmentValues(
      size_t index, const seg_data_t &y0, const seg_data_t &y1);
    
    /** Computes the mean error resulting from approximating a specific 
      * segment.
      *
      * For this, a user-specified error metric is used.
      * \param metric Error metric to be used. See deviation.h for a list
      * of built-in error metrics.
      * \param weights Optional weights table to use for weighting the
      * importance of values.
      * \param seg Segment to compute the error for. This does not need to
      * be one of the segments registered in the LUT.
      */
    deviation_t computeSegmentError(
      error_metric_t metric, WeightsTable *weights, const segment_t &seg);
    /** Wrapper for computeSegmentError accepting a segment index into the
      * list of segments registered in the LUT instead of a segment itself.
      */
    deviation_t computeSegmentError(
      error_metric_t metric, WeightsTable *weights, uint32_t index);
 
    /** Computes the target function with a point in input space.
      */
    void evaluate(const seg_data_t &arg, seg_data_t &res);
    /** Computes the target function with at a point in input space
      */
    seg_data_t evaluate(const seg_data_t &arg);
    
    /** Computes the target function at a point in segment space
      */
    void evaluate(const seg_loc_t &arg, seg_data_t &res);

    /** Computes the target function at a point in segment space
      */
    seg_data_t evaluate(const seg_loc_t &arg);

    /** Computes the target function in hardware space.
      *
      * Hardware space takes a segment index and a word made up of selector
      * and interpolation bits as it would be seen by the HW core's 
      * LUT multiply-add unit. The number of (least significant) bits used in 
      * offset depends on the size of the segment space and can be retrieved by
      * the interpolationBits method.
      *
      * \param seg Segment to use.
      * \param offset Concatenation of selector and interpolation bits
      */
    void evaluate(const segment_t &seg, uint64_t offset, seg_data_t &res);

    /** Computes the target function in hardware space.
      *
      * Hardware space takes a segment index and a word made up of selector
      * and interpolation bits as it would be seen by the HW core's 
      * LUT multiply-add unit. The number of (least significant) bits used in 
      * offset depends on the size of the segment space and can be retrieved by
      * the interpolationBits method.
      *
      * \param seg Segment to use.
      * \param offset Concatenation of selector and interpolation bits
      */

    seg_data_t evaluate(const segment_t &seg, uint64_t offset);
    /** Computes the target function in hardware space.
      *
      * Hardware space takes a segment index and a word made up of selector
      * and interpolation bits as it would be seen by the HW core's 
      * LUT multiply-add unit. The number of (least significant) bits used in 
      * offset depends on the size of the segment space and can be retrieved by
      * the interpolationBits method.
      *
      * \param addr Index of the segment to use.
      * \param offset Concatenation of selector and interpolation bits
      */
    void evaluate(size_t addr, uint64_t offset, seg_data_t &res);

    /** Computes the target function in hardware space.
      *
      * Hardware space takes a segment index and a word made up of selector
      * and interpolation bits as it would be seen by the HW core's 
      * LUT multiply-add unit. The number of (least significant) bits used in 
      * offset depends on the size of the segment space and can be retrieved by
      * the interpolationBits method.
      *
      * \param addr Index of the segment to use.
      * \param offset Concatenation of selector and interpolation bits
      */
    seg_data_t evaluate(size_t addr, uint64_t offset);

    /** Translates our set of segments into an architecture-specific
      * bitstream.
      */
    void translate();
    /** Translates our set of segments into an architecture-specific
      * bitstream.
      */
    void translate2();
    void print_translation_parameters();

};

#endif
