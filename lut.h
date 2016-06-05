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

#include <alpha/alpha.h>

/** represents a single Lookup table.
  *
  * Contains all the information obtained from Input or intermediate files
  * and generated afterwards.
  *
  * All keyvalues read from input files are stored but meaningful ones are
  * interpreted as well and stored in appropriate fields.
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
    alp::array_t<segment_t> _segments;
    
    /** LUT configuration bitstream
      */
    alp::array_t<unsigned char> _config_bits;

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
    const alp::string &fn_weights() const { return _fn_weights; }
    approx_strategy::id_t approximation_strategy() const { 
      return _approximation_strategy; }
    const Bounds &bounds() { return _bounds; }
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
      * @param ptr Pointer into the beginning of the buffer to read from.
      * @param cb Length of the buffer starting at ptr, in bytes.
      * @param name Name used to identify the compilation unit in debug/error
      * output
      */
    void parseInput(const char *ptr, size_t cb, const char *name=NULL);

    /** Parses an input format file using parseInput.
      *
      * @param fn File name of the file to be read
      * @throw FileIOException The given file could not be read.
      */
    void parseInputFile(const char *fn);
    
    
    /** Parses a buffer expected to be in intermediate representation.
      * 
      * Any additional information that was read from previous calls to
      * parseInput, parseIntermediate or was otherwise generated will be lost.
      *
      * @param ptr Pointer into the beginning of the buffer to read from.
      * @param cb Length of the buffer starting at ptr, in bytes.
      */
    void parseIntermediate(const char *ptr, size_t cb, const char *name=NULL);

    /** Parses an intermediate format file using parseIntermediate.
      *
      * @param fn File name of the file to be read
      * @throw FileIOException The given file could not be read.
      * @param name Name used to identify the compilation unit in debug/error
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
    
    const alp::array_t<segment_t> &segments() const { return _segments; }
    /** Adds a new segment into the LUT.
      *
      * \param seg Segment to insert.
      * \param correctOverlap Set to true to adjust other segments to
      * keep the set of segments disjoint. Otherwise it is expected not to
      * overlap with any other segment.
      * \throw RuntimeError The given segment overlaps with another segment
      * and correctOverlap was set to false.
      */
    void addSegment(const segment_t &seg, bool correctOverlap);
    void removeSegment(size_t index) {
      _segments.remove(index);
    }
    void clearSegments() {
      _segments.clear();
    }

    void evaluate(const seg_data_t &arg, seg_data_t &res);
    seg_data_t evaluate(const seg_data_t &arg);


    /** Translates our set of segments into an architecture-specific
      * bitstream.
      */
    void translate();


};

#endif
