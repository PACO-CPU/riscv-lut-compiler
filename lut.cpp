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
  _cmdCompileSO(options_t::Default_cmdCompileSO()),
  _num_segments(arch_config_t::Default_numSegments),
  _num_primary_segments(arch_config_t::Default_numSegments),
  _strategy1(segment_strategy::INVALID),
  _strategy2(segment_strategy::INVALID),
  _explicit_segments(false),
  _bounds(true),
  _target_lib(NULL),
  _target_func(NULL),
  _segment_space_width(-1) {

}
LookupTable::LookupTable(const arch_config_t &cfg) :
  _cmdCompileSO(options_t::Default_cmdCompileSO()),
  _arch(cfg),
  _num_segments(cfg.numSegments),
  _num_primary_segments(cfg.numSegments),
  _strategy1(segment_strategy::INVALID),
  _strategy2(segment_strategy::INVALID),
  _explicit_segments(false),
  _bounds(true), 
  _target_lib(NULL),
  _target_func(NULL),
  _segment_space_width(-1) {

}
LookupTable::LookupTable(const options_t &opts) :
  _cmdCompileSO(opts.cmdCompileSO),
  _arch(opts.arch),
  _num_segments(opts.arch.numSegments),
  _num_primary_segments(opts.arch.numSegments),
  _strategy1(segment_strategy::INVALID),
  _strategy2(segment_strategy::INVALID),
  _explicit_segments(false),
  _bounds(true), 
  _target_lib(NULL),
  _target_func(NULL),
  _segment_space_width(-1) {

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

segment_strategy::id_t LookupTable::ParseSegmentStrategy(const alp::string &s) {
  
  #define SEGMENT_STRATEGY(id,name) \
    } else if (s==name) { \
      return segment_strategy::ID_##id; 

  if (0) {
  #include "strategy-decl.h"
  } else {
    throw SyntaxError("unknown segmentation strategy: "+s);
  }

  #undef SEGMENT_STRATEGY
}

approx_strategy::id_t LookupTable::ParseApproxStrategy(const alp::string &s) {
  
  #define APPROX_STRATEGY(id,name) \
    } else if (s==name) { \
      return approx_strategy::ID_##id;  

  if (0) {
  #include "strategy-decl.h"
  } else {
    throw SyntaxError("unknown approximation strategy: "+s);
  }

  #undef APPROX_STRATEGY
}

void LookupTable::parseInput(const char *ptr, size_t cb, const char *name) {
  InputFlexLexer *lex=InputFlexLexer::New(ptr,cb,name);

  TempDir tempdir;

  enum parse_state_t {
    Keyvalues,
    TargetFunction,
    TargetCode,
    Done
  };
  parse_state_t state=Keyvalues;

  source_location_t loc_target;
  bool identDefined=false;

  
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
        
        // handle special (known) key-values.
        #define KVTEST(id,type) \
          } else if (kv->name()==id) { \
              if (kv->kind()!=KeyValue::type) \
                throw SyntaxError("'" id "' must be a " #type,lex);
        
        if (0) { 
        KVTEST("name",String)
          _ident=kv->val_str();
          identDefined=true;
        KVTEST("num-segments",Integer)
          _num_segments=kv->val_num();
        KVTEST("num-primary-segments",Integer)
          _num_primary_segments=kv->val_num();
        KVTEST("segments",String)
          _strategy1=ParseSegmentStrategy(kv->val_str());
        KVTEST("segments2",String)
          _strategy2=ParseSegmentStrategy(kv->val_str());
        KVTEST("explicit-segments",String)
          _explicit_segments.parse(kv->val_str().ptr,kv->val_str().len);
        KVTEST("weights",String)
          _fn_weights=kv->val_str();
        KVTEST("approximation",String)
          _approximation_strategy=ParseApproxStrategy(kv->val_str());
        KVTEST("bounds",String)
          _bounds.parse(kv->val_str().ptr,kv->val_str().len);
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
  if (!identDefined) {
    throw SyntaxError("no 'name' specified",lex);
  }
  if (_bounds.empty()) {
    throw SyntaxError("no 'bounds' specified",lex);
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
  
  alp::string workdir=tempdir.path();
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
      "%s \"%starget.cpp\" -o \"%s\"",
      _cmdCompileSO.ptr,workdir.ptr,libname.ptr).ptr);

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

  delete lex;

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

void LookupTable::generateOutputFormat(alp::string &res) {
  res.clear();
  res+=alp::string::Format(
    "const char %s_config[%i]=\"",
    _ident.ptr,_config_bits.len);
  
  for(size_t i=0;i<_config_bits.len;i++) {
    res+=alp::string::Format("\\x%.2x",_config_bits[i]);
  }

  res+="\";\n";
}

void LookupTable::saveOutputFile(const char *fn) {
  alp::string data;
  FILE *f;
  generateOutputFormat(data);

  f=fopen(fn,"wb");
  if (!f) throw FileIOException(fn);
  
  if (fwrite(data.ptr,1,data.len,f)<data.len) {
    fclose(f);
    throw FileIOException(fn);
  }

  fclose(f);
}

void LookupTable::computeSegmentSpace() {
  assert(!_bounds.empty() && "Segment space computed without bounds");
  seg_data_t first=_bounds.first(), last=_bounds.last();
  uint64_t width;

  if ((first.kind==seg_data_t::Double)||(last.kind==seg_data_t::Double)) {
    assert(0 && "floating-point input domains not supported");
  }

  width=(uint64_t)last.data_i-(uint64_t)first.data_i+1;

  if (width>(0x80000000uL<<32)) {
    throw RuntimeError("input domain too large: can not exceed 2^63 values");
  }

  for(
    _segment_space_width=0;
    (1uL<<_segment_space_width)<width;
    _segment_space_width++);
  _segment_space_offset=first;

}


void LookupTable::segmentToInputSpace(
  uint32_t segment, double offset, seg_data_t &inp) {

  uint64_t segment_offset = 
    // offset of the segment start from the beginning of the segment space
    (((uint64_t)segment&((1uL<<_arch.selectorBits)-1)) 
    << (_segment_space_width-_arch.selectorBits)) +

    // offset into the segment
    (uint64_t)((1uL<<(_segment_space_width-_arch.selectorBits))*offset);

  // todo: add support for floating-point offsets or emit an error/warning
  inp=(int64_t)segment_offset+_segment_space_offset.data_i;

}
void LookupTable::inputToSegmentSpace(
  uint32_t &segment, double &offset, const seg_data_t &inp) {
  
  uint64_t segment_offset=
    (
      (uint64_t)(inp.data_i-_segment_space_offset.data_i)
      &((1uL<<_segment_space_width)-1));

  segment=segment_offset>>(_segment_space_width-_arch.selectorBits);
  
  segment_offset&=(1uL<<(_segment_space_width-_arch.selectorBits))-1;

  offset=
    (double)segment_offset/
    (double)(1uL<<(_segment_space_width-_arch.selectorBits));

}

#define TEST_BOUNDS(bounds,offset,width,code) { \
 \
  LookupTable lut(opts); \
   \
  const char *input= \
    "name=\"test\" bounds=\"" bounds "\" " \
    "segments=\"uniform\" approximation=\"linear\" " \
    "\n%%\n" \
    "target int->int\n" \
    "\n%%\n" \
    "int target(int a) { return a; }\n" \
    ; \
  lut.parseInput(input,strlen(input),"test lut"); \
  lut.computeSegmentSpace(); \
  Assertf( \
    lut.segment_space_offset()==seg_data_t(offset),\
    "Erroneous segment space offset: %lli, expected %li\n", \
    (int64_t)lut.segment_space_offset(),offset); \
  Assertf( \
    lut.segment_space_width()==width,\
    "Erroneous segment space width: %i, expected %i\n", \
    lut.segment_space_width(),width); \
  code \
}
#define TEST_TRANSL_S2I(segment,offset,pos) { \
  seg_data_t inp; \
  lut.segmentToInputSpace(segment,offset,inp); \
  Assertf( \
    inp==seg_data_t(pos), \
    "Erroneous translation segment space -> input space: (%u,%f) -> %i, " \
    "expected: %li\n", \
    segment,offset, (int)(int64_t)inp, pos); \
}
#define TEST_TRANSL_I2S(pos,segment,offset) { \
  seg_data_t inp(pos); \
  uint32_t _segment; \
  double _offset; \
  lut.inputToSegmentSpace(_segment,_offset,inp); \
  Assertf( \
    (_segment==segment)&&(_offset==offset), \
    "Erroneous translation input -> segment space: %li -> (%u,%f), "\
    "expected: (%u,%f)\n", \
    pos,_segment,_offset, segment,offset); \
}

unittest( 
  /* 
    testing:
      LookupTable::computeSegmentSpace
      LookupTable::segmentToInputSpace
      LookupTable::inputToSegmentSpace
  */

  options_t opts;
  opts.arch.segmentBits=3;
  opts.arch.selectorBits=4;
  
  TEST_BOUNDS( "(1,2)", 1, 1, )
  TEST_BOUNDS( "(13,17)", 13, 3, )
  TEST_BOUNDS( "(127,140) (180,255)", 127, 8, ) 
  TEST_BOUNDS( "(-9,-4) (2,8)", -9, 5, )
  TEST_BOUNDS( "(-17,-4) ", -17, 4, )

  TEST_BOUNDS( "(4,259) ", 4, 8,
    TEST_TRANSL_S2I( 0x0, 0., 4 )
    TEST_TRANSL_S2I( 0x1, 0., 20 )
    TEST_TRANSL_S2I( 0xf, 0., 244 )
    TEST_TRANSL_S2I( 0xf, 0.5, 252 )

    TEST_TRANSL_I2S( 4,   0x0, 0. )
    TEST_TRANSL_I2S( 260, 0x0, 0. )
    TEST_TRANSL_I2S( 300, 0x2, 0.5 )

  )
)
#undef TEST_BOUNDS
#undef TEST_TRANSL_S2I
#undef TEST_TRANSL_I2S


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

void LookupTable::setSegmentValues(
  size_t index, const seg_data_t &y0, const seg_data_t &y1) {
  
  if (index>=_segments.len)
    throw RuntimeError(
      alp::string::Format(
        "Segment index %lu out of bounds [0,%lu)",index,_segments.len));

  _segments[index].y0=y0;
  _segments[index].y1=y1;

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

void LookupTable::evaluate(uint32_t segment, double offset, seg_data_t &res) {
  seg_data_t arg;
  segmentToInputSpace(segment,offset,arg);
  evaluate(arg,res);
}

seg_data_t LookupTable::evaluate(uint32_t segment, double offset) {
  seg_data_t arg,res;
  segmentToInputSpace(segment,offset,arg);
  evaluate(arg,res);
  return res;
}


void LookupTable::translate() {
  // todo: implement 
}
