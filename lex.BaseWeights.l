
%{
#include "weights-lexer.h"
%}

%option c++
%option noyywrap
%option prefix="BaseWeights"
%option yyclass="WeightsFlexLexer"

ws [ \t\r\n]+

integer -?(0[xX][0-9a-fA-F]+|0[0-7]*|[1-9][0-9]*)
float    -?[0-9]*[.][0-9]*([eE][+-]?[0-9]+)?
expression =.*

any (.|[\r\n])

%%

{ws}

{integer} { _numAttr=strtol(yytext,NULL,0); return (_kind=TOK_NUMBER); }
{float} { _numAttr=strtod(yytext,NULL); return (_kind=TOK_NUMBER); }

"(" { return (_kind=TOK_LPAREN); }
")" { return (_kind=TOK_RPAREN); }
"," { return (_kind=TOK_COMMA); }

{expression} { _exprAttr=yytext+1; return (_kind=TOK_EXPRESSION);  }

{any} { return (_kind=TOK_INVALID); }

%%



