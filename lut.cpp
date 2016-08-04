#include "lut.h"

#undef yyFlexLexer
#define yyFlexLexer BaseInputFlexLexer
#include <FlexLexer.h>
#include "input-lexer.h"

#undef yyFlexLexer
#define yyFlexLexer BaseIntermediateFlexLexer
#include <FlexLexer.h>
#include "intermediate-lexer.h"

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

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
        KVTEST("numSegments",Integer)
          _num_segments=kv->val_num().data_i;
        KVTEST("numPrimarySegments",Integer)
          _num_primary_segments=kv->val_num().data_i;
        KVTEST("segments",String)
          _strategy1=ParseSegmentStrategy(kv->val_str());
        KVTEST("segments2",String)
          _strategy2=ParseSegmentStrategy(kv->val_str());
        KVTEST("explicitSegments",String)
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
  bool domainDefined=false;
  segment_t newSegment;

  while(lex->yylex()!=0) {
    switch(lex->kind()) {
      #define ARGUI ({\
        if (lex->yylex()!=IntermediateFlexLexer::TOK_NUMBER) \
          throw SyntaxError("number expected",lex); \
        if (lex->numAttr().kind!=seg_data_t::Integer) \
          throw SyntaxError("integer expected",lex); \
        (uint32_t)lex->numAttr().data_i; \
        })
      #define ARG ({ \
        if (lex->yylex()!=IntermediateFlexLexer::TOK_NUMBER) \
          throw SyntaxError("number expected",lex); \
        lex->numAttr(); \
        })
      #define NEWLINE { \
        if (lex->yylex()!=IntermediateFlexLexer::TOK_NEWLINE) \
          throw SyntaxError("newline expected",lex); \
        }
        
      case IntermediateFlexLexer::TOK_KWNAME:
        if (identDefined) 
          SyntaxWarning("name redefined",lex);
        
        if (lex->yylex()!=IntermediateFlexLexer::TOK_STRING)
          throw SyntaxError("name expected",lex);

        _ident=lex->strAttr();
        
        identDefined=true;

        NEWLINE
        break;

      case IntermediateFlexLexer::TOK_KWSEGMENT:
        if (domainDefined) 
          SyntaxWarning("domain redefined",lex);

        newSegment.width=ARGUI;
        newSegment.prefix=ARGUI;
        newSegment.y0=ARG;
        newSegment.y1=ARG;


        addSegment(newSegment,false);

        NEWLINE 
        break;
      case IntermediateFlexLexer::TOK_KWDOMAIN:
        _segment_space_width=ARGUI;
        _segment_space_offset=ARG;
        
        domainDefined=true;

        NEWLINE
        break;

      case IntermediateFlexLexer::TOK_NEWLINE:
        break;
      default:
        throw SyntaxError("'name' or 'segment' expected",lex);
      #undef ARGUI
      #undef ARG
      #undef NEWLINE
    }
  }

  delete lex;

  if (!identDefined)
    throw SyntaxError("no name defined",lex);
  if (!domainDefined)
    throw SyntaxError("no domain defined",lex);
  
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
  #define addnum(v) \
    switch(v.kind) { \
      case seg_data_t::Integer: \
        res+=alp::string::Format(" %lli",v.data_i); \
        break; \
      case seg_data_t::Double: \
        res+=alp::string::Format(" %lf",v.data_f); \
        break; \
    }
  res.clear();
  res+=alp::string::Format("name \"%s\"\n",_ident.ptr);
  res+=alp::string::Format("domain %u ",_segment_space_width);
  addnum(_segment_space_offset);
  res+="\n";
  for(size_t i=0;i<_segments.len;i++) {
    res+="segment";
    res+=alp::string::Format(" %lu %lu",_segments[i].prefix,_segments[i].width);
    addnum(_segments[i].y0);
    addnum(_segments[i].y1);
    res+="\n";

  }
  #undef addnum
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

  if (_segment_space_width<=_arch.selectorBits) {
    // not really a problem but this makes us an exact map of the specified
    // domain and the programmer might be interested in this fact.
    alp::logf(
      "INFO: input domain does not exceed the number of possible segments\n",
      alp::LOGT_INFO);

    // todo: (7194) Should we really increment the domain size here (to use
    // all possible segments)? Should we perhaps handle this case differently?
    // (no strategies but rather an exact mapping)
    _segment_space_width=_arch.selectorBits;
  }

}

void LookupTable::computePrincipalSegments() {
  
  seg_loc_t loc;
  Bounds::interval_t interval;

  _segments.clear();
  
  for(loc.segment=0;loc.segment<(1L<<_arch.selectorBits);loc.segment++) {
    loc.offset=0;
    segmentToInputSpace(loc,interval.start);
    loc.offset=1;
    segmentToInputSpace(loc,interval.end);
    if (_bounds.intersectsWith(interval))
      _segments.insert(segment_t(loc.segment,1));
  }

}


void LookupTable::segmentToInputSpace(const seg_loc_t &seg, seg_data_t &inp) {

  uint64_t segment_begin = 
    // offset of the segment start from the beginning of the segment space
    (((uint64_t)seg.segment&((1uL<<_arch.selectorBits)-1)) 
    << (_segment_space_width-_arch.selectorBits));
  
  // make it so that an offset of 1 would theoretically be the beginning of
  // the next interval, however we clamp it to a location within our interval.
  uint64_t segment_width=(1uL<<(_segment_space_width-_arch.selectorBits));

  uint64_t segment_offset=0;// offset into the segment
  
  // skip negative, NaN and -inf (default to zero)
  if (seg.offset>0) {
    segment_offset=(uint64_t)(segment_width*seg.offset);
    if (segment_offset>=segment_width) segment_offset=segment_width-1;
  }

  // todo: add support for floating-point offsets or emit an error/warning
  inp=
    (int64_t)segment_begin+(int64_t)segment_offset+
    _segment_space_offset.data_i;

}
void LookupTable::inputToSegmentSpace(seg_loc_t &seg, const seg_data_t &inp) {
  
  uint64_t segment_offset=
    (
      (uint64_t)(inp.data_i-_segment_space_offset.data_i)
      &((1uL<<_segment_space_width)-1));

  seg.segment=segment_offset>>(_segment_space_width-_arch.selectorBits);
  
  segment_offset&=(1uL<<(_segment_space_width-_arch.selectorBits))-1;

  seg.offset=
    (double)segment_offset/
    (double)(1uL<<(_segment_space_width-_arch.selectorBits));

}

void LookupTable::hardwareToInputSpace(
  const segment_t &seg, uint64_t offset, seg_data_t &inp) {

  // determine where in the input word our interpolation bits reside.
  int offsetShift=0;
  int interpolationBits=
    _segment_space_width-_arch.selectorBits;

  if (interpolationBits>_arch.interpolationBits) {
    offsetShift=interpolationBits-_arch.interpolationBits;
    interpolationBits=_arch.interpolationBits;
  }

  
  // clamp offset to the number of data points in the segment
  if (offset>=(((uint64_t)seg.width)<<interpolationBits)) {
    offset=(((uint64_t)seg.width)<<interpolationBits)-1;
  }
  // shift offset to the correct level in our segment space
  if (offsetShift>0) offset<<= offsetShift;
  
  // move the offset relative to the segment beginning
  offset+=seg.prefix<<(_segment_space_width-_arch.selectorBits);
  
  inp=_segment_space_offset+seg_data_t((int64_t)offset);

}


void LookupTable::hardwareToInputSpace(
  size_t addr, uint64_t offset, seg_data_t &inp) {

  if (addr>=_segments.len)
    throw RuntimeError(
      alp::string::Format(
        "Segment index %lu out of bounds [0,%lu)",addr,_segments.len));
  
  segment_t &seg=_segments[addr];

  hardwareToInputSpace(seg,offset,inp);
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
  lut.computePrincipalSegments(); \
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
  seg_loc_t seg(segment,offset); \
  lut.segmentToInputSpace(seg,inp); \
  Assertf( \
    inp==seg_data_t(pos), \
    "Erroneous translation segment space -> input space: (%u,%f) -> %i, " \
    "expected: %li\n", \
    segment,offset, (int)(int64_t)inp, pos); \
}
#define TEST_TRANSL_I2S(pos,_segment,_offset) { \
  seg_data_t inp(pos); \
  seg_loc_t seg; \
  lut.inputToSegmentSpace(seg,inp); \
  Assertf( \
    (seg.segment==_segment)&&(seg.offset==_offset), \
    "Erroneous translation input -> segment space: %li -> (%u,%f), "\
    "expected: (%u,%f)\n", \
    pos,seg.segment,seg.offset, _segment,_offset); \
}
#define TEST_TRANSL_H2I(prefix,width,offset,pos) { \
  segment_t seg(prefix,width); \
  seg_data_t inp; \
  lut.hardwareToInputSpace(seg,offset,inp); \
  Assertf( \
    inp==seg_data_t(pos), \
    "Erroneous translation segment space -> input space: (%u,%u, %u) -> %i, " \
    "expected: %li\n", \
    prefix,width,offset, (int)(int64_t)inp, pos); \
}
#define TEST_SEGMENTS(code) { \
  alp::array_t<segment_t> segments=lut.segments().dup(); \
  code \
  for(size_t i=0;i<segments.len;i++) \
    Assertf( \
      segments.len==0, \
      "Stray segment: %i %i \n",segments[i].prefix,segments[i].width); \
}
#define SEGMENT(prefix,width) { \
  int idx; \
  if ((idx=segments.find_eq(segment_t(prefix,width)))<0) \
    Assertf( \
      false, \
      "Missing expected segment (prefix: %i, width: %i)\n", \
      prefix,width); \
  else \
    segments.remove(idx); \
}
unittest( 
  /* 
    testing:
      LookupTable::computeSegmentSpace
      LookupTable::segmentToInputSpace
      LookupTable::inputToSegmentSpace
      LookupTable::computePrincipalSegments
  */

  options_t opts;
  opts.arch.segmentBits=3; // up to eight possible segments will be used

  opts.arch.selectorBits=1; // two different principal segments exist
  TEST_BOUNDS( "(1,2)", 1, 1, )

  opts.arch.selectorBits=4; // sixteen different principal segments exist
  TEST_BOUNDS( "(1,2)", 1, 4, )
  TEST_BOUNDS( "(13,17)", 13, 4, )
  TEST_BOUNDS( "(127,140) (180,255)", 127, 8, ) 
  TEST_BOUNDS( "(-9,-4) (2,8)", -9, 5, )
  TEST_BOUNDS( "(-17,-4) ", -17, 4, )

  TEST_BOUNDS( "(4,259) ", 4, 8,
    TEST_TRANSL_S2I( 0x0, 0., 4 )
    TEST_TRANSL_S2I( 0x1, 0., 20 )
    TEST_TRANSL_S2I( 0x7, 0., 116 )
    TEST_TRANSL_S2I( 0x7, 0.5, 124 )

    TEST_TRANSL_I2S( 4,   0x0, 0. )
    TEST_TRANSL_I2S( 260, 0x0, 0. )
    TEST_TRANSL_I2S( 300, 0x2, 0.5 )

    TEST_TRANSL_H2I( 1, 1, 0, 20 )
    
    TEST_SEGMENTS(
      for (size_t i=0;i<16;i++)
        SEGMENT(i,1)
    )
  )

  TEST_BOUNDS( "(0,32) (254,255)", 0,8,
    TEST_SEGMENTS(
      SEGMENT(0,1)
      SEGMENT(1,1)
      SEGMENT(2,1)
      SEGMENT(15,1)
    )
  )

)
#undef TEST_BOUNDS
#undef TEST_TRANSL_S2I
#undef TEST_TRANSL_I2S
#undef TEST_SEGMENTs
#undef SEGMENT


bool LookupTable::addSegment(const segment_t &seg, bool failOnOverlap) {
  size_t idx;
  for (idx=0;idx<_segments.len;idx++) {
    if (_segments[idx].prefix>=seg.prefix+seg.width) break; // we precede 
    if (_segments[idx].prefix+_segments[idx].width<=seg.prefix) 
      continue; // we succede

    if (!failOnOverlap)
      throw RuntimeError(
        alp::string::Format(
          "the new segment (%u:%u) overlaps with another one",
          seg.prefix,seg.prefix+seg.width));

    return false;
  }
  _segments.insert(seg,idx);
  return true;
}


bool LookupTable::addSegment(
  const seg_data_t &x0, const seg_data_t &x1, bool failOnOverlap) {
  
  segment_t seg;
  seg_loc_t loc0,loc1;
  
  inputToSegmentSpace(loc0,x0);
  inputToSegmentSpace(loc1,x1);

  if (loc0.segment>loc1.segment) return false;

  seg.prefix=loc0.segment;
  seg.width=loc1.segment-loc0.segment+1;

  return addSegment(seg,failOnOverlap);


}

bool LookupTable::addSegment(
  uint32_t prefix, uint32_t width, bool failOnOverlap) {

  uint32_t num_principal_segments=1uL<<_arch.selectorBits;
  
  if (prefix>=num_principal_segments) return false;
  if (prefix+width>num_principal_segments) width=num_principal_segments-prefix;
  
  if (width<1) return false;
  segment_t seg;

  seg.prefix=prefix;
  seg.width=width;

  return addSegment(seg,failOnOverlap);
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

deviation_t LookupTable::computeSegmentError(
  error_metric_t metric, WeightsTable *weights, const segment_t &seg) {
  
  uint64_t point_count=((uint64_t)seg.width)<<segment_interpolation_bits();
  seg_data_t x_raw,y_raw,weight_raw;
  double
    sum_w=0,
    sum_e=0,
    base=seg.y0,
    incline=
      ((double)seg.y1-(double)seg.y0)/
      (double)((point_count<1)?1:point_count-1);

  for(uint64_t x=0;x<point_count;x++) {
    double w=1,y,e;
    hardwareToInputSpace(seg,x,x_raw);
    evaluate(seg,x,y_raw);
    if (weights!=NULL) {
      weights->evaluate(x_raw,weight_raw);
      w=(double)weight_raw;
    }
    
    y=(double)y_raw;
    e=base+x*incline-y;

    sum_e+=e*e*w;
    sum_w+=w;
  }
  if (sum_w<=0) return deviation_t(0,0);

  return deviation_t(sum_e/sum_w,sum_w);
}

deviation_t LookupTable::computeSegmentError(
  error_metric_t metric, WeightsTable *weights, uint32_t index) {
  
  const segment_t &seg=_segments[index];
  return computeSegmentError(metric,weights,seg);
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

void LookupTable::evaluate(const seg_loc_t &arg, seg_data_t &res) {
  seg_data_t arg_i;
  segmentToInputSpace(arg,arg_i);
  evaluate(arg_i,res);
}

seg_data_t LookupTable::evaluate(const seg_loc_t &arg) {
  seg_data_t arg_i,res;
  segmentToInputSpace(arg,arg_i);
  evaluate(arg_i,res);
  return res;
}

void LookupTable::evaluate(
  const segment_t &seg, uint64_t offset, seg_data_t &res) {
  seg_data_t arg_i;
  hardwareToInputSpace(seg,offset,arg_i);
  evaluate(arg_i,res);
}

seg_data_t LookupTable::evaluate(
  const segment_t &seg, uint64_t offset) {
  seg_data_t arg_i,res;
  hardwareToInputSpace(seg,offset,arg_i);
  evaluate(arg_i,res);
  return res;
}

void LookupTable::evaluate(size_t addr, uint64_t offset, seg_data_t &res) {
  seg_data_t arg_i;
  hardwareToInputSpace(addr,offset,arg_i);
  evaluate(arg_i,res);
}

seg_data_t LookupTable::evaluate(size_t addr, uint64_t offset) {
  seg_data_t arg_i,res;
  hardwareToInputSpace(addr,offset,arg_i);
  evaluate(arg_i,res);
  return res;
}

void LookupTable::print_translation_parameters(){
  // num of the PLA inputs (inverted not included)
  printf("selectorBits: %i\n", _arch.selectorBits);
  // number of wires between AND and OR plane of the PLA
  printf("_arch.plaInterconnects: %i\n", _arch.plaInterconnects );
  // width of the PLA output
  printf(
    "_arch.segmentBits: %i (2^segmentBits: # of possible different MAu inputs)\n",
    _arch.segmentBits);
  // assert that number of segments does not exceed log2(segmentBits),
  // meaning the number of LUT memory slots is sufficient
  assert( (_segments.len >> _arch.segmentBits) == 0 &&
          "translate: #of segments exceeds LUT memory slots");
  // number of input bits that the LUT multiply/add unit (MAu) uses by design
  printf("interpolationBits: %i\n", _arch.interpolationBits);
  // number of input bits that will used in this configuration (<= previous)
  printf("segment_interpolation_bits: %i\n", segment_interpolation_bits());
  // no idea!
  printf("_num_segments: %i\n", _num_segments);
  // no idea!
  printf("_num_primary_segments: %i\n", _num_primary_segments);
  // offset of the beginning of the first segment from input space, integer
  printf("_segment_space_offset: %li\n", _segment_space_offset.data_i );
  // log2 of the width of the input domain
  printf("_segment_space_width: %i\n", _segment_space_width );
}

  /**
    * Shift a bitvector left through a uint64_t array.
    */
void LookupTable::bitvector_leftshift( uint64_t* bitvector, int len, int* current_bits,
                            int additional_bits){
    if((*current_bits/64)<((*current_bits+additional_bits)/64)){
      for( int j=len-1; j>0; j--){
        bitvector[j] = (bitvector[j] << additional_bits) +
                           (bitvector[j-1] >> (additional_bits-64));
      }
        bitvector[0] = (bitvector[0] << additional_bits);
    } else {
      for( int j=len-1; j>=0; j--){
        bitvector[j] = (bitvector[j] << additional_bits);
      }
    }
    *current_bits += additional_bits;
  }

void LookupTable::translate() {
  /** 
    * The translation function translates an arithmetical specification
    * of segments delivered by the compiler frontend into binary bitstreams
    * that can be transferred to the LUT hardware in the pipeline via DMA.
    * There, these bitstreams define the behaviour of the address translation
    * PLA and provide output values for each segment either directly or via
    * the LUT units inbuilt Multiply-Add unit.
    */

  // do a sanity check on the LUT description
  assert( _segments.len > 0 && "translate: #of segments not larger than 0");
  
  //print_translation_parameters();

  // Variables of interest during translation
  const int REG_WIDTH = 64;

  // Allocate char arrays to hold config information.
  // And plane of the PLA; *2: for inverted inputs
  and_plane_conf = new char[_arch.plaInterconnects*2*_arch.selectorBits];
  // Or plane of the PLA
  or_plane_conf = new char[_arch.plaInterconnects*_arch.segmentBits];
  // Connection plane connecting suitable input lines to the PLA
  connection_plane_conf = new char[3*REG_WIDTH*(_arch.selectorBits
				       +_arch.interpolationBits)];
  // Array of LUT outputs used as factors in the Multiply-Add unit
  factor_output = new seg_data_t[1<<_arch.segmentBits];
  // Array of LUT outputs used as offsets in the Multiply-Add unit
  offset_output = new seg_data_t[1<<_arch.segmentBits];
  // TODO: for now, only linear interpolation, no steps
  use_multiply_add = true;


  /* Calculate LUT decoder configuration
     - calculate *which* bits of the input are used for selection and
       interpolation.
     - calculate MinTerms for And plane of PLA
     - Or plane: naive addresses for now
   */
  // 1. connection plane
  // Connect interpolationBits for Multiply-Add unit
  // Which LUT input bits are used for interpolation within segments?
  // Counting from MSBs:
  //   The selectorBits and following until a number of interpolationBits
  //   is reached.
  int interpolate_LSB = _segment_space_width - _arch.interpolationBits;
  int interpolate_MSB = _segment_space_width;
  //printf("interpolateLSB: %i, interpolateMSB: %i\n",
  //       interpolate_LSB, interpolate_MSB);
  // Connect selectorBits to PLA via connection plane
  for( int interpolateBit = 0; interpolateBit<_arch.interpolationBits; 
       interpolateBit++){
    for( int registerBit = 0; registerBit<REG_WIDTH*3; registerBit++){
      if( interpolate_LSB <= registerBit
          && registerBit < interpolate_MSB  
          && registerBit - interpolate_LSB == interpolateBit){ // connection
        //printf("connected: regbit: %i, interpolatebit: %i\n",
        //        registerBit, interpolateBit);
        connection_plane_conf[interpolateBit*REG_WIDTH*3+registerBit] = true;
      }else{ // no connection
        connection_plane_conf[interpolateBit*REG_WIDTH*3+registerBit] = false;
      }
    }
  }
  // Which LUT input bits are used for segment selection?
  // Counting from LSBs:
  //   (_segment_space_width-_arch.selectorBits) to _segment_space_width
  int select_LSB = _segment_space_width - _arch.selectorBits;
  int select_MSB = _segment_space_width;
  //printf("selLSB: %i, selMSB: %i\n", select_LSB, select_MSB);
  int sco = _arch.interpolationBits*REG_WIDTH*3; //segment configuration offset
  // Connect selectorBits to PLA via connection plane
  for( int selectorBit = 0; selectorBit<_arch.selectorBits; selectorBit++){
    for( int registerBit = 0; registerBit<REG_WIDTH*3; registerBit++){ // 3 rs
      if( select_LSB <= registerBit
          && registerBit < select_MSB  
          && registerBit - select_LSB == selectorBit){ // connection
        //printf("connected: regbit: %i, selbit: %i\n", registerBit, selectorBit);
        connection_plane_conf[selectorBit*REG_WIDTH*3+registerBit+sco] = true;
      }else{ // no connection
        connection_plane_conf[selectorBit*REG_WIDTH*3+registerBit+sco] = false;
      }
    }
  }

  // 2. PLA -- MinTerms and naive addresses
  int current_interconnect = 0;
  for( size_t current_segment = 0; current_segment < _segments.len;
       current_segment++){
    qmc_pla_gen( &current_interconnect, current_segment,
                 &_segments[current_segment],
                 and_plane_conf, or_plane_conf, _arch.selectorBits,
                 _arch.segmentBits);
  }
  // write impossible MinTerms to rest of and_plane_conf, initialize or_plane
  for(; current_interconnect<_arch.plaInterconnects; current_interconnect++){
    for(int i=0;i<_arch.selectorBits;i++) {
      and_plane_conf[current_interconnect*2*_arch.selectorBits+i]=true;
      and_plane_conf[(current_interconnect*2+1)*_arch.selectorBits+i]=true;
    }
    for(int i=0;i<_arch.segmentBits;i++) {
      or_plane_conf[current_interconnect*_arch.segmentBits+i] = true;
    }
  }
 
//  switch( _segments[0].y0.kind) {
//    case seg_data_t::Integer:
//      for( size_t i=0; i<_segments.len; i++) {
//        printf( "Prefix: %u Width: %u\n", _segments[i].prefix, _segments[i].width);
//        printf( "y0: %li, y1: %li\n", _segments[i].y0.data_i, _segments[i].y1.data_i);
//      }
//      break;
//    case seg_data_t::Double:
//        // todo: handle FP
//        assert(0 && "Floating-Point translation not implemented");
//      break;
//  }

  // Encode to alp::array_t<unsigned char> _config_bits
  /* Encode in order:
       - RAM1, RAM2, ..., RAMn
       - PLAo, PLAo-1, ..., PLA1
       - IDECm, IDECm-1, ..., IDEC1
       - shifter ?
       - comparator (one line only) ?
   */
  // RAM
  // in every bitvector: lower bits are incline, upper are base
  uint64_t* RAM_bitvector;
  int64_t slope;
  int64_t normalized_slope;
  // create bitvector, rounding words up.
  int rbs = (_arch.incline_bits+_arch.base_bits+63)/64; //RAM bitvector size
  int rbcl; // current length of the bitvector in bits
  RAM_bitvector = new uint64_t[rbs];
  for( size_t i=0; i<_segments.len; i++) {
    for( int j=0; j<rbs; j++) RAM_bitvector[j]=0;
    // create base first, left-shift it
    RAM_bitvector[0] = ((1uL<<_arch.base_bits)-1) & (uint32_t)_segments[i].y0.data_i;
    rbcl = _arch.base_bits;
    //printf("base RAM Bitvector: %lx \n", RAM_bitvector[0]);
    // shift through processor words, if needed
    bitvector_leftshift( RAM_bitvector, rbs, &rbcl, _arch.incline_bits);
    //printf("shifted RAM Bitvector: %lx \n", RAM_bitvector[0]);
    // normalize for interpolationBits
    // TODO: When shifter HW is implemented, replace this.
    slope = ( _segments[i].y1.data_i - _segments[i].y0.data_i +
              _arch.interpolationBits / 2) /_arch.interpolationBits;
    //printf("slope: %lx \n", slope);
    //printf("(_segment_space_width-_arch.interpolationBits): %u \n", (_segment_space_width-_arch.interpolationBits));
    normalized_slope = slope << (_segment_space_width-_arch.interpolationBits);
    //printf("shifted slope: %lx \n", normalized_slope);
    RAM_bitvector[0] += ((1uL<<_arch.incline_bits)-1) & (uint64_t)normalized_slope;
  }

  // PLA, first all AND then OR
  // AND: 2*selectorBits length, configure lowest interconnect (AND1) first,
  //      first bit: LSB connectionPlane, ..., MSB connectionPlane, inverted ..
  uint64_t* AND_bitvector;
  // create bitvector, rounding words up.
  int abs = (2*_arch.selectorBits+63)/64; //AND bitvector size
  int abcl; // current length of the bitvector in bits
  AND_bitvector = new uint64_t[abs];
  for( int i=0; i<_arch.plaInterconnects; i++) {
    for( int j=0; j<abs; j++) AND_bitvector[j]=0;
    // start from MSB from connectionPlane, then push through
    abcl = 0;
    for( int j=2*_arch.selectorBits-1; j>=0; j--){
      // shift left, through processor words if needed
      bitvector_leftshift( AND_bitvector, abs, &abcl, 1);
      if(and_plane_conf[i*2*_arch.selectorBits+j]){
        AND_bitvector[0] += 1;
        //printf("1");
      } else {
        //printf("0");
      }
      //printf("bitvector: " BYTE_TO_BINARY_PATTERN " " BYTE_TO_BINARY_PATTERN "\n",
      //BYTE_TO_BINARY((AND_bitvector[0])>>8), BYTE_TO_BINARY(AND_bitvector[0]));
    }
    //printf(", %d\n", i);
    //printf("AND bitvector: " BYTE_TO_BINARY_PATTERN " " BYTE_TO_BINARY_PATTERN "\n",
    //BYTE_TO_BINARY((AND_bitvector[0])>>8), BYTE_TO_BINARY(AND_bitvector[0]));
  }

  // OR: number of interconnects length, first bit for each vector is for AND1
  uint64_t* OR_bitvector;
  // create bitvector, rounding words up.
  int obs = (_arch.plaInterconnects+63)/64; //OR bitvector size
  int obcl; // current length of the bitvector in bits
  OR_bitvector = new uint64_t[obs];
  for( int i=0; i<_arch.segmentBits; i++) {
    for( int j=0; j<obs; j++) OR_bitvector[j]=0;
    // start from last AND, then push through
    obcl = 0;
    for( int j=_arch.plaInterconnects-1; j>=0; j--){
      // shift left, through processor words if needed
      bitvector_leftshift( OR_bitvector, obs, &obcl, 1);
      if(or_plane_conf[j*_arch.segmentBits+i]){
        OR_bitvector[0] += 1;
        //printf("1");
      } else {
        //printf("0");
      }
      //printf("bitvector: " BYTE_TO_BINARY_PATTERN " " BYTE_TO_BINARY_PATTERN "\n",
      //BYTE_TO_BINARY((AND_bitvector[0])>>8), BYTE_TO_BINARY(AND_bitvector[0]));
    }
    //printf(", %d\n", i);
    //printf("AND bitvector: " BYTE_TO_BINARY_PATTERN " " BYTE_TO_BINARY_PATTERN "\n",
    //BYTE_TO_BINARY((AND_bitvector[0])>>8), BYTE_TO_BINARY(AND_bitvector[0]));
  }

  // connection plane, most significant register bit first, starting with rs3
  // first: selectorBits configuration, push
  // second: interpolationBits configuration
  uint64_t* connect_bitvector;
  // create bitvector, rounding words up.
  int cbs = (REG_WIDTH*3+63)/64; //connection bitvector size
  int cbcl; // current length of the bitvector in bits
  connect_bitvector = new uint64_t[cbs];
  for( int i=0; i<_arch.interpolationBits+_arch.selectorBits; i++) {
    for( int j=0; j<cbs; j++) connect_bitvector[j]=0;
    // start from MSB of rs3, then push through
    cbcl = 0;
    for( int j=REG_WIDTH*3-1; j>=0; j--){
      // shift left, through processor words if needed
      bitvector_leftshift( connect_bitvector, cbs, &cbcl, 1);
      if(connection_plane_conf[i*REG_WIDTH*3+j]){
        connect_bitvector[0] += 1;
        //printf("1");
      } else {
        //printf("0");
      }
      //printf("bitvector: " BYTE_TO_BINARY_PATTERN " " BYTE_TO_BINARY_PATTERN "\n",
      //BYTE_TO_BINARY((AND_bitvector[0])>>8), BYTE_TO_BINARY(AND_bitvector[0]));
    }
    //printf(", %d\n", i);
    //printf("AND bitvector: " BYTE_TO_BINARY_PATTERN " " BYTE_TO_BINARY_PATTERN "\n",
    //BYTE_TO_BINARY((AND_bitvector[0])>>8), BYTE_TO_BINARY(AND_bitvector[0]));
  }
  /* TODO: the bitvectors
             - RAM_bitvector
             - OR_bitvector
             - AND_bitvector
             - connect_bitvector
           must be written to a file of the compiler developer's choosing.
  */

  // Shifter
  //   unknown if implemented
  // comparator (one line only)
  //   unknown if implemented
}
