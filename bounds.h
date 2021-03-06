#ifndef RISCV_LUT_COMPULER_BOUNDS_H
#define RISCV_LUT_COMPULER_BOUNDS_H

#include "error.h"
#include "segment.h"
#include "util.h"

#include <alpha/alpha.h>

#undef yyFlexLexer
#define yyFlexLexer BaseBoundsFlexLexer
#include <FlexLexer.h>
#include "bounds-lexer.h"

/** Represents a set of intervals for use in input domain specification.
  *
  * Internally this is stored as a sorted array of disjoint intervals.
  */
class Bounds {
  public:
    /** Represents a single interval. */
    struct interval_t {
      seg_data_t start, end;
    };
  protected:
    alp::array_t<interval_t> _data;
    bool _autoMerge;

  public:
    Bounds(bool autoMerge);
    /** Getter for the sorted list of disjoint intervals making up this
      * set of bounds. */
    const alp::array_t<interval_t> &data() const { return _data; }
    
    /** Adds an interval to the bounds.
      *
      * This will shrink all intervals that overlap with the new one,
      * potentially deleting intervals that are covered completely.
      */
    void addInterval(interval_t ival);

    /** Removes all intervals, making this object represent the empty set.
      */
    void clear() { _data.clear(); }

    /** Returns true iff this bounds objects contains no points.
      *
      * That is, it has no intervals or all of them are empty.
      */
    bool empty();
    
    /** Returns the lower bound of the first interval */
    seg_data_t first() const {
      if (_data.len<1) return (int64_t)0;
      return _data[0].start;
    }
    /** Returns the upper bound of the last interval */
    seg_data_t last() const {
      if (_data.len<1) return (int64_t)0;
      return _data[_data.len-1].end;
    }
    
    /** Returns true iff the given interval intersects with this one.
      *
      * An intersection occurs if there is such a pair (a,b) of intervals,
      * with a from us and b from ival so that at least one point of a and b
      * are the same.
      */
    bool intersectsWith(const interval_t &ival) const;

    /** Parses a string expected to be of format:
      *        ( '(' number ',' number ')' ) *
      */
    void parse(const char *ptr, size_t cb);

    /** Same as the other parse method but inputs a lexer instance.
      *
      * Used to call the parsing of bounds out of a greater context,
      * e.g. segmentation strategies which allow for explicit bounds
      * specification.
      *
      * This method expects the lexer to have already sanned the opening
      * parenthesis (lex->kind() equals TOK_LPAREN).
      */
    void parse(BoundsFlexLexer *lex);
};

#endif
