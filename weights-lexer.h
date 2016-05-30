/** \file weights-lexer.h
  * \brief Lexer class for lexing weights files.
  */
#ifndef RISCV_LUT_COMPILER_WEIGHTS_LEXER_H
#define  RISCV_LUT_COMPILER_WEIGHTS_LEXER_H

#include "segment.h"
#include <alpha/alpha.h>

class WeightsFlexLexer : public BaseWeightsFlexLexer {
  public:
    enum {
      TOK_INVALID=1,
      TOK_LPAREN,
      TOK_RPAREN,
      TOK_COMMA,
      TOK_NUMBER,
      TOK_EXPRESSION,
    };
  protected:
    int _kind;
    seg_data_t _numAttr;
    alp::string _exprAttr;

  public:
    
    WeightsFlexLexer( std::istream *arg_yyin=0, std::ostream *arg_yyout=0) :
      BaseWeightsFlexLexer(arg_yyin,arg_yyout), 
      _kind(TOK_INVALID) {
    }

    virtual int yylex();

    int kind() const { return _kind; }
    const seg_data_t &numAttr() { return _numAttr; }
    const alp::string &exprAttr() { return _exprAttr; }
};

#endif
