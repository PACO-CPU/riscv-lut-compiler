
%{
#include "weights-lexer.h"
#define EMIT_SYMBOL(tok) { \
  _loc_last=_loc; \
  _loc+=strlen(yytext);  \
  return (_kind=tok); \
}
%}

%option c++
%option noyywrap
%option prefix="BaseWeights"
%option yyclass="WeightsFlexLexer"

ws [ \t]+
nl [\r]?[\n]

integer -?(0[xX][0-9a-fA-F]+|0[0-7]*|[1-9][0-9]*)
float    -?[0-9]*[.][0-9]*([eE][+-]?[0-9]+)?
expression =.*

any (.|[\r\n])

%%

{ws}      { _loc+=strlen(yytext); }
{nl}      { _loc.newline(strlen(yytext)); }

{integer} { 
  _loc_last=_loc; 
  _loc+=strlen(yytext); 
  _numAttr=strtol(yytext,NULL,0); 
  return (_kind=TOK_NUMBER); 
}
{float}   { 
  _loc_last=_loc; 
  _loc+=strlen(yytext); 
  _numAttr=strtod(yytext,NULL); 
  return (_kind=TOK_NUMBER); 
}

"(" { EMIT_SYMBOL(TOK_LPAREN); }
")" { EMIT_SYMBOL(TOK_RPAREN); }
"," { EMIT_SYMBOL(TOK_COMMA); }

{expression} { 
  _loc_last=_loc; 
  _loc+=strlen(yytext); 
  _exprAttr=yytext+1; 
  return (_kind=TOK_EXPRESSION);  
}

{any} { 
  _loc_last=_loc; 
  _loc+=strlen(yytext); 
  return (_kind=TOK_INVALID); 
}

%%



