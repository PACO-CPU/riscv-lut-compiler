
%{
#include "intermediate-lexer.h"
#define EMIT_SYMBOL(tok) { \
  _loc_last=_loc; \
  _loc+=strlen(yytext);  \
  return (_kind=tok); \
}
%}

%option c++
%option noyywrap
%option prefix="BaseIntermediate"
%option yyclass="IntermediateFlexLexer"

ws [ \t]+
nl [\r]?[\n]
nl_wrap [\\][\r]?[\n]

ident ([_a-zA-Z])[_a-zA-Z0-9]*
string \"([^"]+|[\]["])*\"
sstring [^" \t\r\n][^ \t\r\n]*
integer -?(0[xX][0-9a-fA-F]+|0[0-7]*|[1-9][0-9]*)
float    -?[0-9]*[.][0-9]*([eE][+-]?[0-9]+)?

any .

%%



{ws}      { _loc+=strlen(yytext); }
{nl_wrap} { _loc.newline(strlen(yytext)); }
{nl}      { _loc.newline(strlen(yytext)); return (_kind=TOK_NEWLINE); }

"name" { EMIT_SYMBOL(TOK_KWNAME); }
"segment" { EMIT_SYMBOL(TOK_KWSEGMENT); }

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
{sstring} {
  _loc_last=_loc;
  _loc+=strlen(yytext);
  _strAttr=yytext;
  return (_kind=TOK_STRING);
}
{string}  {
  _loc_last=_loc; 
  _loc+=strlen(yytext); 
  _strAttr=alp::string::Unescape(yytext+1,strlen(yytext)-2); 
  return (_kind=TOK_STRING); 
}


{any} { 
  _loc_last=_loc; 
  _loc+=strlen(yytext); 
  return (_kind=TOK_INVALID); 
}

%%



