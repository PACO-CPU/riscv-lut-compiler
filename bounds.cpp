#include "bounds.h"
#include "error.h"



Bounds::Bounds(bool autoMerge) : _autoMerge(autoMerge) {
}

void Bounds::addInterval(interval_t ival) {
  if (ival.end<ival.start) return;
  size_t i;
  for(i=0;i<_data.len;i++) {
    if (_data[i].end<ival.start) continue;
    if (_data[i].start>ival.end) break;
    if (
      !_autoMerge && 
      ((_data[i].end>ival.start) || (_data[i].start<ival.end))) {
      throw RuntimeError(
        alp::string::Format(
          "overlapping intervals: (%g %g) and (%g %g)",
          (double)_data[i].start,(double)_data[i].end,
          (double)ival.start,(double)ival.end));
    }
    if (_data[i].end>=ival.end) {
      _data[i].start=ival.start;
      return;
    } else {
      if (_data[i].start<ival.start) ival.start=_data[i].start;
      _data.remove(i);
      i--;
    }
  }
  _data.insert(ival,i);
}


void Bounds::parse(const char *ptr, size_t cb) {
  _data.clear();
   
  BoundsFlexLexer *lex=BoundsFlexLexer::New(ptr,cb,"");
  
  segment_t newSegment;

  lex->yylex();
  parse(lex);

  delete lex;


}

void Bounds::parse(BoundsFlexLexer *lex) {
  interval_t newInterval;

  while(lex->kind()!=0) {
    switch(lex->kind()) {
      case BoundsFlexLexer::TOK_LPAREN:
        
        if (lex->yylex()!=BoundsFlexLexer::TOK_NUMBER)
          throw SyntaxError("number expected",lex);
        newInterval.start=lex->numAttr();

        if (lex->yylex()!=BoundsFlexLexer::TOK_COMMA)
          throw SyntaxError("',' expected",lex);
        
        if (lex->yylex()!=BoundsFlexLexer::TOK_NUMBER)
          throw SyntaxError("number expected",lex);
        newInterval.end=lex->numAttr();

        if (lex->yylex()!=BoundsFlexLexer::TOK_RPAREN)
          throw SyntaxError("')' expected",lex);

        addInterval(newInterval);
        break;
      default:
        printf("%i",lex->kind());
        throw SyntaxError("'(' expected",lex);
    }
    if (lex->yylex()==0) break;
  }
} 
