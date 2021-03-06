#ifndef RISCV_LUT_COMPILER_WEIGHTS_H
#define  RISCV_LUT_COMPILER_WEIGHTS_H

#include "error.h"
#include "segment.h"
#include "bounds.h"
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}
/** Represents a single weights table as specified in weights data files.
  
  */
class WeightsTable : public alp::RCObject {
  public:
    /** Represents a single entry in the weights file.
      */
    struct range_t {
      /** Start of the range */
      seg_data_t start;
      /** End of the range. 
        *
        * If this is less than or equal to the start, this range represents
        * a point.
        */
      seg_data_t end;
      /** Lua function to call with a point from this range to attain the value
        * at that point.
        */
      int    lref;

      /** The weights table this range originally comes from.
        */
      WeightsTable *owner;
      

      bool contains(const seg_data_t &p) {
        return ((start<=p) && (end>=p));
      }


    };
  protected:
    /** Holds true iff all interval boundaries are integers. */
    bool _isAllIntegers;

    /** The lua state used for interpreting our segment data points */
    lua_State *_l;
    
    /** Sorted list of our ranges.
      *
      * To uphold the overriding nature of succeeding ranges in weight files,
      * overlaps are resolved and intervals are split accordingly whenever
      * a new range is added.
      */
    alp::array_t<range_t> _ranges;
  public:
    
    /** (Copy) constructor
      *
      * Creates a deep duplicate of the passed argument if not NULL
      */
    WeightsTable(WeightsTable *tbl=NULL);
    
    /** Constructor creating a weights table equaling 1 for all points 
      * in the passed bounds object.
      */
    WeightsTable(const Bounds &bounds);

    virtual ~WeightsTable();
    
    /** Removes all ranges, leaving the weights table as it was right after
      * construction.*/
    void clear();
   
    /** Returns true iff all the range boundaries are integers */
    bool isAllIntegers() const { return _isAllIntegers;}
    /** Parses a weights file and *adds* the defined ranges to our own.
      *
      * \param ptr Pointer to the beginning of the buffer to parse.
      * \param cb Length of the buffer to parse, in bytes.
      */
    void parseWeights(const char *ptr, size_t cb, const char *name=NULL);
    /** Uses parseWeights to parse a weights file.
      *
      * \param fn File name to load and parse from. 
      * \throw FileIOException The file could not be read.
      * \param name Name used to identify the compilation unit in debug/error
      * output
      */
    void parseWeightsFile(const char *fn);
   
    /** Returns the lower bound of the first interval */
    seg_data_t first() const {
      if (_ranges.len<1) return (int64_t)0;
      return _ranges[0].start;
    }
    /** Returns the upper bound of the last interval */
    seg_data_t last() const {
      if (_ranges.len<1) return (int64_t)0;
      return _ranges[_ranges.len-1].end;
    }
    
    /** Sets a given range to zero by resizing/removing any ranges that 
      * intersect.
      */
    void setZeroRange(const seg_data_t &start, const seg_data_t &end);

    /** Returns the weight value at a specific point.
      * 
      * \param p Point to compute the weight at.
      */
    seg_data_t evaluate(const seg_data_t &p);
    /** Call-by-reference version of evaluate
      * 
      * \param p Point to compute the weight at.
      * \param r Result variable.
      */
    void evaluate(const seg_data_t &p, seg_data_t &v);

};

#endif
