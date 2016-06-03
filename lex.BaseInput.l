
%{
#include "input-lexer.h"
%}

%option c++
%option noyywrap
%option prefix="BaseInput"
%option yyclass="InputFlexLexer"

ws [ \t]+
nl [\r]?[\n]
emptyLine [\r\n][ \t]*[\r\n]

ident ([_a-zA-Z])[_a-zA-Z0-9]*
string \"([^"]+|[\]["])*\"
integer -?(0[xX][0-9a-fA-F]+|0[0-7]*|[1-9][0-9]*)
float    -?[0-9]*[.][0-9]*([eE][+-]?[0-9]+)?

any (.|[\r\n])

%%



{emptyLine} { 
  _loc_last=_loc; _loc.newline(strlen(yytext),2);
  return (_kind=TOK_EMPTY_LINE); 
}

{ws}      { _loc+=strlen(yytext); }
{nl}      { _loc.newline(strlen(yytext)); }
{ident}   { 
  _loc_last=_loc; 
  _loc+=strlen(yytext); 
  _strAttr=yytext; 
  return (_kind=TOK_IDENT); 
}
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
{string}  {
  _loc_last=_loc; 
  _loc+=strlen(yytext); 
  _strAttr=alp::string::Unescape(yytext+1,strlen(yytext)-2); 
  return (_kind=TOK_STRING); 
}

"=" { 
  _loc_last=_loc; 
  _loc+=strlen(yytext); 
  return (_kind=TOK_EQUALS); 
}

{any} { 
  _loc_last=_loc; 
  _loc+=strlen(yytext); 
  return (_kind=TOK_INVALID); 
}

%%



