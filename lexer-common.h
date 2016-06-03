#ifndef RISCV_LUT_COMPILER_LEXER_COMMON_H
#define  RISCV_LUT_COMPILER_LEXER_COMMON_H

#include <stdlib.h>
/** Structure holding a source location during parsing.
  */
struct source_location_t {
  /** Current index of the line, starting at 1 */
  int lidx;
  /** Current index of the character within the line, starting at 0.
    *
    * This counts *raw* characters, not utf8 symbols.
    */
  int cidx;

  /** Index into the raw character buffer where we currently are.
    */
  size_t raw_offset;

  source_location_t() : lidx(1), cidx(0), raw_offset(0) {

  }
  
  void operator+=(int nchr) {
    cidx+=nchr;
    raw_offset+=nchr;
  }

  void newline(int nchr, int count=1) {
    lidx+=count;
    cidx=0;
    raw_offset+=nchr;
  }

};

/** Interface for lexers to allow error handling access to source locations.*/
class SourceLocationLexer {
  public:
    virtual const source_location_t &loc()=0;
    virtual const char *ptr()=0;
    virtual size_t      cb()=0;
    virtual const char *unitName()=0;
};

#endif
