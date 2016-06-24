#include "weights.h"
#include "util.h"

#undef yyFlexLexer
#define yyFlexLexer BaseWeightsFlexLexer
#include <FlexLexer.h>
#include "weights-lexer.h"

#include <stdio.h>
#include <math.h>

#include <iostream>
WeightsTable::WeightsTable(WeightsTable *tbl) :
  _isAllIntegers(true) {
  
  // creates the math library as a new table (and leaves it on the stack)
  luaL_requiref(_l,LUA_MATHLIBNAME,luaopen_math,1);

  // iterate through all members..
  lua_pushnil(_l);
  while(lua_next(_l,-2)!=0) {
    lua_setglobal(_l,lua_tostring(_l,-2)); // and make them global
  }
  lua_pop(_l,1);

  if (tbl!=NULL) {
    _isAllIntegers=tbl->_isAllIntegers;
    for(size_t i=0;i<tbl->_ranges.len;i++) {
      _ranges.insert(tbl->_ranges[i]);
      tbl->grab();
    }
  }

}

WeightsTable::~WeightsTable() {
  for(size_t i=0;i<_ranges.len;i++) {
    if (_ranges[i].owner!=this) _ranges[i].owner->drop();
  }
  // no need to free any ranges as the array_t is self-cleaning and
  // references to lua objects get freed by lua_close.
  lua_close(_l);
}

void WeightsTable::clear() {
  _isAllIntegers=true;
  for(size_t i=0;i<_ranges.len;i++) {
    luaL_unref(_ranges[i].owner->_l,LUA_REGISTRYINDEX,_ranges[i].lref);
    if (_ranges[i].owner!=this) _ranges[i].owner->drop();
  }
  _ranges.clear();
}

void WeightsTable::parseWeights(const char *ptr, size_t cb, const char *name) {
  WeightsFlexLexer *lex=WeightsFlexLexer::New(ptr,cb,name);

  range_t newRange;
  newRange.owner=this;
  
  while(lex->yylex()!=0) {
    switch(lex->kind()) {
      case WeightsFlexLexer::TOK_LPAREN: // ( number , number ) = expression
        if (lex->yylex()!=WeightsFlexLexer::TOK_NUMBER)
          throw SyntaxError("number expected",lex);
        newRange.start=lex->numAttr();
        if (lex->yylex()!=WeightsFlexLexer::TOK_COMMA)
          throw SyntaxError("',' expected",lex);
        if (lex->yylex()!=WeightsFlexLexer::TOK_NUMBER)
          throw SyntaxError("number expected",lex);
        newRange.end=lex->numAttr();
        if (lex->yylex()!=WeightsFlexLexer::TOK_RPAREN)
          throw SyntaxError("')' expected",lex);

        break;

      case WeightsFlexLexer::TOK_NUMBER: // number = expression
        newRange.start=newRange.end=lex->numAttr();
        break;

      default:
        throw SyntaxError("number or '(' expected",lex);
    }


    if (lex->yylex()!=WeightsFlexLexer::TOK_EXPRESSION)
      throw SyntaxError("expression expected",lex);
    
    const alp::string expr="return "+lex->exprAttr();
    size_t idx;
    
    if (luaL_loadbuffer(_l,expr.ptr,expr.len,"")!=0) {
      alp::string msg=lua_tostring(_l,-1);
      lua_pop(_l,1);
      throw SyntaxError("Lua error: "+msg,lex);
    }
    newRange.lref=luaL_ref(_l,LUA_REGISTRYINDEX);

    for (idx=0;idx<_ranges.len;idx++) {
      if (_ranges[idx].start>newRange.end) break; // we precede 
      if (_ranges[idx].end<newRange.start) continue; // we succede
      if (_ranges[idx].start<newRange.end) { 
        // we supersede the ending -> crop the other one
        _ranges[idx].end=newRange.start;

      } else if (_ranges[idx].end>newRange.end) {
        // we supersede the beginning -> crop the other one, done
        _ranges[idx].start=newRange.end;

        break; // we cannot touch the next one
      } else {
        // we overlap completely -> remove the other one
        luaL_unref(_ranges[idx].owner->_l,LUA_REGISTRYINDEX,_ranges[idx].lref);
        if (_ranges[idx].owner!=this) _ranges[idx].owner->drop();
        _ranges.remove(idx);
        idx--;
      }
    }
    
    _ranges.insert(newRange,idx);

    if (
      _isAllIntegers && (
        (newRange.start.kind!=seg_data_t::Integer) ||
        (newRange.end.kind  !=seg_data_t::Integer))) 
      _isAllIntegers=false;

  }

  delete lex;

}

void WeightsTable::parseWeightsFile(const char *fn) {
  FILE *f=fopen(fn,"rb");
  char *buf;
  size_t cb;
  if (!f) throw FileIOException(fn);

  fseek(f,0,SEEK_END);
  cb=ftell(f);
  fseek(f,0,SEEK_SET);

  buf=(char*)malloc(cb);
  if (buf==NULL) {
    fclose(f);
    throw FileIOException(fn);
  }

  if (fread(buf,1,cb,f)<cb) {
    fclose(f);
    throw FileIOException(fn);
  }
  fclose(f);

  try {
    parseWeights(buf,cb,fn);
  } catch(SyntaxError &e) {
    free((void*)buf);
    throw e;
  }
  free((void*)buf);
}

void WeightsTable::setZeroRange(
  const seg_data_t &start, const seg_data_t &end) {
  size_t idx;

  for (idx=0;idx<_ranges.len;idx++) {
    if (_ranges[idx].start>end) break; // we precede 
    if (_ranges[idx].end<start) continue; // we succede
    if (_ranges[idx].start<end) { 
      // we supersede the ending -> crop the other one
      _ranges[idx].end=start;

    } else if (_ranges[idx].end>end) {
      // we supersede the beginning -> crop the other one, done
      _ranges[idx].start=end;

      break; // we cannot touch the next one
    } else {
      // we overlap completely -> remove the other one
      luaL_unref(_ranges[idx].owner->_l,LUA_REGISTRYINDEX,_ranges[idx].lref);
        if (_ranges[idx].owner!=this) _ranges[idx].owner->drop();
      _ranges.remove(idx);
      idx--;
    }
  }
}

seg_data_t WeightsTable::evaluate(const seg_data_t &p) {
  seg_data_t r;
  evaluate(p,r);
  return r;
}

void WeightsTable::evaluate(const seg_data_t &p, seg_data_t &v) {
  ssize_t l,r,c;
  lua_Number lv;
  
  for(l=0,r=(ssize_t)_ranges.len-1;l<=r;) {
    c=(l+r)/2;
    if (_ranges[c].contains(p)) {
      lua_rawgeti(_ranges[c].owner->_l,LUA_REGISTRYINDEX,_ranges[c].lref);
      switch(p.kind) {
        case seg_data_t::Integer:
          lua_pushnumber(_l,p.data_i);
          break;
        case seg_data_t::Double:
          lua_pushnumber(_l,p.data_f);
          break;
      }
      lua_setglobal(_l,"v");
      if (lua_pcall(_l,0,1,0)!=0) {
        // error: ignore and return 0
        lua_pop(_l,0);
        break;
      }
      lv=lua_tonumber(_l,-1);
      lua_pop(_l,-1);
      if (floor(lv)==lv) v=(int64_t)floor(lv);
      else v=lv;
      return;
    }
    if (_ranges[c].start>p) r=c-1;
    else l=c+1;
  }

  v=(int64_t)0;
}
