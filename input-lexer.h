/** \file input-lexer.h
  * \brief Lexer class for lexing input files.
  */
#ifndef RISCV_LUT_COMPILER_INPUT_LEXER_H
#define  RISCV_LUT_COMPILER_INPUT_LEXER_H

#include "segment.h"
#include "lexer-common.h"
#include "util.h"
#include <alpha/alpha.h>

class InputFlexLexer : public BaseInputFlexLexer, public SourceLocationLexer {
  public:
    enum {
      TOK_INVALID=1,
      TOK_EQUALS,
      TOK_ARROW,
      TOK_SEPARATOR,
      TOK_IDENT,
      TOK_NUMBER,
      TOK_STRING,
    };
  protected:
    /** Current source location of the scanner.
      */
    source_location_t _loc;
    /** Source location of the beginning of the currently displayed token */
    source_location_t _loc_last;
    int _kind;

    seg_data_t _numAttr;
    alp::string _strAttr;

    const char *_code;
    size_t      _cb_code;
    const char *_name;

    membuf_read_t *_in_buf;
    std::istream *_in_stream;

    InputFlexLexer( 
      membuf_read_t *in_buf, std::istream *in_stream, 
      const char *code, size_t cb, const char *name) :
      BaseInputFlexLexer(in_stream,NULL),
      _kind(TOK_INVALID),
      _code(code),
      _cb_code(cb),
      _name(name),
      _in_buf(in_buf),
      _in_stream(in_stream) {
    }
  public:
    

    ~InputFlexLexer() {
      delete _in_stream;
      delete _in_buf;
    }

    static InputFlexLexer *New(const char *ptr, size_t cb, const char *name) {
      membuf_read_t *in_buf=new membuf_read_t(ptr,cb);
      std::istream *in_stream=new std::istream(in_buf);
      InputFlexLexer *lex=new InputFlexLexer(in_buf,in_stream,ptr,cb,name);
      return lex;
    }
    
    virtual int yylex();

    int kind() const { return _kind; }
    const seg_data_t &numAttr() { return _numAttr; }
    const alp::string &strAttr() { return _strAttr; }
    virtual const source_location_t &loc() { return _loc_last; }
    const source_location_t &loc_cur() { return _loc; }
    virtual const char *ptr() { return _code; };
    virtual size_t      cb() { return _cb_code; };
    virtual const char *unitName() { return _name; };

};

#endif
