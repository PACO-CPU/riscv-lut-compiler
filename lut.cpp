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

  if (_segment_space_width<=_arch.segmentBits) {
    // not really a problem but this makes us an exact map of the specified
    // domain and the programmer might be interested in this fact.
    alp::logf(
      "INFO: input domain does not exceed the number of segments\n",
      alp::LOGT_INFO);

    // todo: (7194) Should we really increment the domain size here (to use
    // all possible segments)? Should we perhaps handle this case differently?
    // (no strategies but rather an exact mapping)
    _segment_space_width=_arch.segmentBits;
  }

}

void LookupTable::computePrincipalSegments() {
  
  seg_loc_t loc;
  Bounds::interval_t interval;

  _segments.clear();
  
  for(loc.segment=0;loc.segment<(1L<<_arch.segmentBits);loc.segment++) {
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
    (((uint64_t)seg.segment&((1uL<<_arch.segmentBits)-1)) 
    << (_segment_space_width-_arch.segmentBits));
  
  // make it so that an offset of 1 would theoretically be the beginning of
  // the next interval, however we clamp it to a location within our interval.
  uint64_t segment_width=(1uL<<(_segment_space_width-_arch.segmentBits));

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

  seg.segment=segment_offset>>(_segment_space_width-_arch.segmentBits);
  
  segment_offset&=(1uL<<(_segment_space_width-_arch.segmentBits))-1;

  seg.offset=
    (double)segment_offset/
    (double)(1uL<<(_segment_space_width-_arch.segmentBits));

}

void LookupTable::hardwareToInputSpace(size_t addr, uint64_t offset, seg_data_t &inp) {
  if (addr>=_segments.len)
    throw RuntimeError(
      alp::string::Format(
        "Segment index %lu out of bounds [0,%lu)",addr,_segments.len));
  
  segment_t &seg=_segments[addr];

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
  opts.arch.selectorBits=4;

  opts.arch.segmentBits=1; // two segments will be generated
  TEST_BOUNDS( "(1,2)", 1, 1, )

  opts.arch.segmentBits=3; // eight segments will be generated
  TEST_BOUNDS( "(1,2)", 1, 3, )
  TEST_BOUNDS( "(13,17)", 13, 3, )
  TEST_BOUNDS( "(127,140) (180,255)", 127, 8, ) 
  TEST_BOUNDS( "(-9,-4) (2,8)", -9, 5, )
  TEST_BOUNDS( "(-17,-4) ", -17, 4, )

  TEST_BOUNDS( "(4,259) ", 4, 8,
    TEST_TRANSL_S2I( 0x0, 0., 4 )
    TEST_TRANSL_S2I( 0x1, 0., 36 )
    TEST_TRANSL_S2I( 0x7, 0., 228 )
    TEST_TRANSL_S2I( 0x7, 0.5, 244 )

    TEST_TRANSL_I2S( 4,   0x0, 0. )
    TEST_TRANSL_I2S( 260, 0x0, 0. )
    TEST_TRANSL_I2S( 300, 0x1, 0.25 )
    
    TEST_SEGMENTS(
      for (size_t i=0;i<8;i++)
        SEGMENT(i,1)
    )
  )

  TEST_BOUNDS( "(0,32) (254,255)", 0,8,
    TEST_SEGMENTS(
      SEGMENT(0,1)
      SEGMENT(1,1)
      SEGMENT(7,1)
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

  uint32_t num_principal_segments=1uL<<_arch.segmentBits;
  
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

void LookupTable::translate() {
  // todo: implement 
}
