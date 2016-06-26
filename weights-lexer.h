/** \file weights-lexer.h
  * \brief Lexer class for lexing weights files.
  */
#ifndef RISCV_LUT_COMPILER_WEIGHTS_LEXER_H
#define  RISCV_LUT_COMPILER_WEIGHTS_LEXER_H

#include "segment.h"
#include "lexer-common.h"
#include "util.h"
#include <alpha/alpha.h>

/** Scanner for domain specification intervals.
  *
  * This is a wrapper for the flex-generated BaseBoundsFlexLexer, adding
  * token kinds, attributes and source locations as well as a wrapper
  * constructor for creating a scanner from a char pointer.
  */
class WeightsFlexLexer : 
  public BaseWeightsFlexLexer, public SourceLocationLexer {
  public:
    /** Token kind enumeration
      */
    enum {
      TOK_INVALID=1,
      TOK_LPAREN,
      TOK_RPAREN,
      TOK_COMMA,
      TOK_NUMBER,
      TOK_EXPRESSION,
    };
  protected:
    /** Current source location of the scanner.
      */
    source_location_t _loc;
    /** Source location of the beginning of the currently displayed token */
    source_location_t _loc_last;
    int _kind;
    
    /** Numeric attribute data. */
    seg_data_t _numAttr;
    /** String-typed attribute data. */
    alp::string _exprAttr;
    
    /** Raw code as specified with the constructor. */
    const char *_code;
    /** Length, in bytes, of the source code we are scanning.*/
    size_t      _cb_code;
    /** Identifying name of the source code block we are scanning.*/
    const char *_name;

    membuf_read_t *_in_buf;
    std::istream *_in_stream;
    
    /** Protected constructor. This is needed to ensure integrity of data
      * generated in the wrapper New.
      */    
    WeightsFlexLexer( 
      membuf_read_t *in_buf, std::istream *in_stream, 
      const char *code, size_t cb, const char *name) :
      BaseWeightsFlexLexer(in_stream,NULL),
      _kind(TOK_INVALID),
      _code(code),
      _cb_code(cb),
      _name(name),
      _in_buf(in_buf),
      _in_stream(in_stream) {
    }
  public:
    

    ~WeightsFlexLexer() {
      delete _in_stream;
      delete _in_buf;
    }

    /** Wrapper for the constructor, accepting a raw character buffer as input
      */
    static WeightsFlexLexer *New(const char *ptr, size_t cb, const char *name) {
      membuf_read_t *in_buf=new membuf_read_t(ptr,cb);
      std::istream *in_stream=new std::istream(in_buf);
      WeightsFlexLexer *lex=new WeightsFlexLexer(in_buf,in_stream,ptr,cb,name);
      return lex;
    }
    
    /** Performs a single scanning step, returning on EOF or when a new 
      * token was scanned.
      *
      * \return The token kind as described in the enum above.
      */
    virtual int yylex();
    
    /** Returns the token kind emitted by the last call to yylex. */
    int kind() const { return _kind; }
    const seg_data_t &numAttr() { return _numAttr; }
    const alp::string &exprAttr() { return _exprAttr; }
    virtual const source_location_t &loc() { return _loc_last; }
    const source_location_t &loc_cur() { return _loc; }
    virtual const char *ptr() { return _code; };
    virtual size_t      cb() { return _cb_code; };
    virtual const char *unitName() { return _name; };
};

#endif
