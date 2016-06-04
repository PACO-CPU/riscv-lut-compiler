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
    const alp::array_t<interval_t> &data() { return _data; }

    void addInterval(interval_t ival);
    void clear() { _data.clear(); }
    
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
