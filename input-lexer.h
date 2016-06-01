/** \file input-lexer.h
  * \brief Lexer class for lexing input files.
  */
#ifndef RISCV_LUT_COMPILER_INPUT_LEXER_H
#define  RISCV_LUT_COMPILER_INPUT_LEXER_H

#include "segment.h"
#include <alpha/alpha.h>

class InputFlexLexer : public BaseInputFlexLexer {
  public:
    enum {
      TOK_INVALID=1,
      TOK_EQUALS,
      TOK_EMPTY_LINE,
      TOK_IDENT,
      TOK_NUMBER,
      TOK_STRING,
    };
  protected:
    int _kind;
    seg_data_t _numAttr;
    alp::string _strAttr;

  public:
    
    InputFlexLexer( std::istream *arg_yyin=0, std::ostream *arg_yyout=0) :
      BaseInputFlexLexer(arg_yyin,arg_yyout), 
      _kind(TOK_INVALID) {
    }

    virtual int yylex();

    int kind() const { return _kind; }
    const seg_data_t &numAttr() { return _numAttr; }
    const alp::string &strAttr() { return _strAttr; }
};

#endif
