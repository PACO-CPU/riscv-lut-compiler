#include "lut.h"

#undef yyFlexLexer
#define yyFlexLexer BaseInputFlexLexer
#include <FlexLexer.h>
#include "input-lexer.h"

#undef yyFlexLexer
#define yyFlexLexer BaseIntermediateFlexLexer
#include <FlexLexer.h>
#include "intermediate-lexer.h"

LookupTable::LookupTable() :
  _target_lib(NULL),
  _target_func(NULL) {

}

LookupTable::~LookupTable() {
  if (_target_lib!=NULL)
    dlib_close(_target_lib);
}

ssize_t LookupTable::findKeyValue(const alp::string &key) const {
  for(size_t i=0;i<_keyvalues.len;i++) 
    if (_keyvalues[i]->name()==key) return i;
  return -1;
}

KeyValue *LookupTable::getKeyValue(const alp::string &key) const {
  for(size_t i=0;i<_keyvalues.len;i++) 
    if (_keyvalues[i]->name()==key) return _keyvalues[i];
  return NULL;
}

void LookupTable::parseInput(const char *ptr, size_t cb, const char *name) {
  InputFlexLexer *lex=InputFlexLexer::New(ptr,cb,name);

  enum parse_state_t {
    Keyvalues,
    TargetFunction,
    TargetCode,
    Done
  };
  parse_state_t state=Keyvalues;

  source_location_t loc_target;
  
  // parse keyvalues
  while(state==Keyvalues) {
    switch(lex->yylex()) {
      case InputFlexLexer::TOK_SEPARATOR: 
        state=TargetFunction;
        break;
      case InputFlexLexer::TOK_IDENT: {
        alp::string name;
        ssize_t idx0;

        KeyValue *kv=NULL;

        name=lex->strAttr();
        loc_target=lex->loc();
        
        if (lex->yylex()!=InputFlexLexer::TOK_EQUALS)
          throw SyntaxError("'=' expected",lex);

        switch(lex->yylex()) {
          case InputFlexLexer::TOK_NUMBER:
            kv=new KeyValue(name,lex->numAttr());
            break;
          case InputFlexLexer::TOK_STRING:
            kv=new KeyValue(name,lex->strAttr());
            break;
          default:
            throw SyntaxError("string or number expected",lex);
        }

        if ((idx0=findKeyValue(name))>-1) {
          delete _keyvalues[idx0];
          _keyvalues.remove(idx0);
        }

        _keyvalues.insert(kv);
        break;
      }
      default:
        throw SyntaxError("key-value or separator expected",lex);
    }
  }
  // parse target function
  while(state==TargetFunction) {
    switch(lex->yylex()) {
      case InputFlexLexer::TOK_IDENT: {
        target_type_t type;
        
        _target_name=lex->strAttr();
        
        while(lex->yylex()!=InputFlexLexer::TOK_ARROW) {
          if (lex->kind()!=InputFlexLexer::TOK_IDENT)
            throw SyntaxError("type name expected",lex);
          if (!type.set(lex->strAttr().ptr))
            throw SyntaxError("invalid type name",lex);
          _target_argument_types.insert(type);
        }

        if (lex->yylex()!=InputFlexLexer::TOK_IDENT)
          if (lex->kind()!=InputFlexLexer::TOK_IDENT)
            throw SyntaxError("type name expected",lex);
        if (!type.set(lex->strAttr().ptr))
          throw SyntaxError("invalid type name",lex);
        _target_result_type=type;


        if (lex->yylex()!=InputFlexLexer::TOK_SEPARATOR)
          throw SyntaxError("'%%' expected",lex);
        state=TargetCode;
        break;
      }
      default:
        throw SyntaxError("identifier expected",lex);
    }
  }

  // parse target code
  while(state==TargetCode) {
    size_t offs=lex->loc_cur().raw_offset;
    _c_code=alp::string(ptr+offs,cb-offs);
    state=Done;
  }

  delete lex;
  

  if (_target_argument_types.len!=1) 
    throw SyntaxError(
      "invalid number of arguments for target function",lex,loc_target);
  
  /* compile c code:
    seg_data_t definition
    target code
    wrapper code
    */

  // todo: automatically generate (AND cleanup) a temporary directory
  alp::string workdir="/tmp/lut_test/";
  alp::string libname=workdir+"target.so";
  FILE *f;
  int r;
  
  f=fopen((workdir+"target.cpp").ptr,"w");
  
  // seg_data_t definition
  fprintf(f,"%s",SEG_DATA_DECL);

  // target code
  fwrite(_c_code.ptr,1,_c_code.len,f);

  /* wrapper code:
    void compute_target(seg_data_t *res, const seg_data_t *arg0 ,...) {
      *res=<target_name>(*arg0,...);
    }
    */
  fprintf(f,
    "\n"
    "void compute_target(seg_data_t *res");
  for(size_t i=0;i<_target_argument_types.len;i++)
    fprintf(f,", const seg_data_t *arg%lu",i);
  fprintf(f,
    ") {\n  *res=%s(*arg0",
    _target_name.ptr);

  for(size_t i=1;i<_target_argument_types.len;i++) fprintf(f,",*arg%lu",i);

  fprintf(f,
    ");\n}\n");
  
  fclose(f);

  // load c code
  
  // fixme: this needs to be tested on windows. mingw-gcc -shared *should*
  // output a DLL so other than the extension it *should* work fine
  r=system(
    alp::string::Format(
      "gcc %starget.cpp -g -fPIC -shared -o%s",
      workdir.ptr,libname.ptr).ptr);

  if (r!=0) 
    throw RuntimeError(
      "target library compilation failed");

  _target_lib=dlib_open(libname.ptr);
  if (_target_lib==NULL)
    throw RuntimeError(
      alp::string::Format("unable to load library file <%s>",libname.ptr));
  
  _target_func=(target_func_t)dlib_lookup(_target_lib,"_Z14compute_targetP10seg_data_tPKS_");
  if (_target_func==NULL) {
    dlib_close(_target_lib);
    _target_lib=NULL;
    throw RuntimeError(
      alp::string::Format(
        "unable to locate target function in library file <%s>",libname.ptr));
  }


}

void LookupTable::parseInputFile(const char *fn) {
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
    parseInput(buf,cb,fn);
  } catch(SyntaxError &e) {
    free((void*)buf);
    throw e;
  }
  free((void*)buf);
}

void LookupTable::parseIntermediate(
  const char *ptr, size_t cb, const char *name) {

  IntermediateFlexLexer *lex=IntermediateFlexLexer::New(ptr,cb,name);
  
  bool identDefined=false;
  segment_t newSegment;

  while(lex->yylex()!=0) {
    switch(lex->kind()) {
      case IntermediateFlexLexer::TOK_KWNAME:
        if (identDefined)
          throw SyntaxError("name redefined",lex);
        
        if (lex->yylex()!=IntermediateFlexLexer::TOK_STRING)
          throw SyntaxError("name expected",lex);

        _ident=lex->strAttr();

        if (lex->yylex()!=IntermediateFlexLexer::TOK_NEWLINE)
          throw SyntaxError("newline expected",lex);
        
        identDefined=true;
        break;

      case IntermediateFlexLexer::TOK_KWSEGMENT:
        #define ARG(id) \
          if (lex->yylex()!=IntermediateFlexLexer::TOK_NUMBER) \
            throw SyntaxError("number expected",lex); \
          newSegment.id=lex->numAttr();

        ARG(x0)
        ARG(x1)
        ARG(y0)
        ARG(y1)

        #undef ARG

        addSegment(newSegment,false);

        if (lex->yylex()!=IntermediateFlexLexer::TOK_NEWLINE)
          throw SyntaxError("newline expected",lex);
        
        break;
      case IntermediateFlexLexer::TOK_NEWLINE:
        break;
      default:
        throw SyntaxError("'name' or 'segment' expected",lex);
    }
  }

  if (!identDefined)
    throw SyntaxError("no name defined",lex);
  
}
void LookupTable::parseIntermediateFile(const char *fn) {
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
    parseIntermediate(buf,cb,fn);
  } catch(SyntaxError &e) {
    free((void*)buf);
    throw e;
  }
  free((void*)buf);
}

void LookupTable::addSegment(const segment_t &seg, bool correctOverlap) {
  size_t idx;
  for (idx=0;idx<_segments.len;idx++) {
    if (_segments[idx].x0>seg.x1) break; // we precede 
    if (_segments[idx].x1<seg.x0) continue; // we succede

    if (!correctOverlap)
      throw RuntimeError(
        alp::string::Format(
          "the new segment (%g:%g) overlaps with another one",
          (double)seg.x0,(double)seg.x1));
    
    if (_segments[idx].x0<seg.x1) {
      // we supersede the ending -> crop the other one
      _segments[idx].x1=seg.x0;

    } else if (_segments[idx].x1>seg.x1) {
      // we supersede the beginning -> crop the other one, done
      _segments[idx].x0=seg.x1;

      break; // we cannot touch the next one
    } else {
      // we overlap completely -> remove the other one
      _segments.remove(idx);
      idx--;
    }
  }
  _segments.insert(seg,idx);
}

void LookupTable::generateIntermediateFormat(alp::string &res) {
  res.clear();
  res+=alp::string::Format("name \"%s\"\n",_ident.ptr);
  for(size_t i=0;i<_segments.len;i++) {
    res+="segment";
    #define addnum(v) \
      switch(v.kind) { \
        case seg_data_t::Integer: \
          res+=alp::string::Format(" %lli",v.data_i); \
          break; \
        case seg_data_t::Double: \
          res+=alp::string::Format(" %lf",v.data_f); \
          break; \
      }
    addnum(_segments[i].x0);
    addnum(_segments[i].x1);
    addnum(_segments[i].y0);
    addnum(_segments[i].y1);
    res+="\n";

  }
}

void LookupTable::saveIntermediateFile(const char *fn) {
  alp::string data;
  FILE *f;
  generateIntermediateFormat(data);

  f=fopen(fn,"wb");
  if (!f) throw FileIOException(fn);
  
  if (fwrite(data.ptr,1,data.len,f)<data.len) {
    fclose(f);
    throw FileIOException(fn);
  }

  fclose(f);
}

void LookupTable::evaluate(const seg_data_t &arg, seg_data_t &res) {
  // if _target_func is NULL, the caller was not careful enough.
  assert( (_target_func!=NULL) && "Target function was not loaded" );
  _target_func(&res,&arg);
}

seg_data_t LookupTable::evaluate(const seg_data_t &arg) {
  seg_data_t res;
  evaluate(arg,res);
  return res;
}
