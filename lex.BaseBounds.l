
%{
#include "bounds-lexer.h"
#define EMIT_SYMBOL(tok) { \
  _loc_last=_loc; \
  _loc+=strlen(yytext);  \
  return (_kind=tok); \
}
%}

%option c++
%option noyywrap
%option prefix="BaseBounds"
%option yyclass="BoundsFlexLexer"

ws [ \t]+
nl [\r]?[\n]

ident ([_a-zA-Z])[_a-zA-Z0-9]*
integer -?(0[xX][0-9a-fA-F]+|0[0-7]*|[1-9][0-9]*)
float    -?[0-9]*[.][0-9]*([eE][+-]?[0-9]+)?

any .

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
{ident}  {
  _loc_last=_loc; 
  _loc+=strlen(yytext); 
  _strAttr=yytext;
  return (_kind=TOK_IDENT); 
}

"(" { EMIT_SYMBOL(TOK_LPAREN); }
")" { EMIT_SYMBOL(TOK_RPAREN); }
"," { EMIT_SYMBOL(TOK_COMMA); }

{any} { 
  _loc_last=_loc; 
  _loc+=strlen(yytext); 
  return (_kind=TOK_INVALID); 
}

%%



