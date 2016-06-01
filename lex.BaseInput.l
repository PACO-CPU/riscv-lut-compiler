
%{
#include "input-lexer.h"
%}

%option c++
%option noyywrap
%option prefix="BaseInput"
%option yyclass="InputFlexLexer"

ws [ \t\r\n]+
emptyLine [\r\n][ \t]*[\r\n]

ident ([_a-zA-Z])[_a-zA-Z0-9]*
string \"([^"]+|[\]["])*\"
integer -?(0[xX][0-9a-fA-F]+|0[0-7]*|[1-9][0-9]*)
float    -?[0-9]*[.][0-9]*([eE][+-]?[0-9]+)?

any (.|[\r\n])

%%

{emptyLine} { return (_kind=TOK_EMPTY_LINE); }
{ws}
{ident} { _strAttr=yytext; return (_kind=TOK_IDENT); }
{integer} { _numAttr=strtol(yytext,NULL,0); return (_kind=TOK_NUMBER); }
{float} { _numAttr=strtod(yytext,NULL); return (_kind=TOK_NUMBER); }
{string} { 
  _strAttr=alp::string::Unescape(yytext+1,strlen(yytext)-2); 
  return (_kind=TOK_STRING); 
}

"=" { return (_kind=TOK_EQUALS); }

{any} { return (_kind=TOK_INVALID); }

%%



