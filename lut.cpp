#include "lut.h"
#include "util.h"

#undef yyFlexLexer
#define yyFlexLexer BaseInputFlexLexer
#include <FlexLexer.h>
#include "input-lexer.h"

LookupTable::LookupTable() {

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


void LookupTable::parseInput(const char *ptr, size_t cb) {
  membuf_read_t in_buf=membuf_read_t(ptr,cb);
  std::istream *in_stream=new std::istream(&in_buf);
  InputFlexLexer *lex=new InputFlexLexer(in_stream);
  
  bool done=false;
  
  while(!done && (lex->yylex()!=0)) {
    printf("%i\n",lex->kind());
    switch(lex->kind()) {
      case InputFlexLexer::TOK_IDENT: {
        alp::string name;
        ssize_t idx0;

        KeyValue *kv=NULL;

        name=lex->strAttr();
        
        if (lex->yylex()!=InputFlexLexer::TOK_EQUALS)
          throw SyntaxError("'=' expected");

        switch(lex->yylex()) {
          case InputFlexLexer::TOK_NUMBER:
            kv=new KeyValue(name,lex->numAttr());
            break;
          case InputFlexLexer::TOK_STRING:
            kv=new KeyValue(name,lex->strAttr());
            break;
          default:
            throw SyntaxError("string or number expected");
        }

        if ((idx0=findKeyValue(name))>-1) {
          delete _keyvalues[idx0];
          _keyvalues.remove(idx0);
        }

        _keyvalues.insert(kv);
        break;
      }
      case InputFlexLexer::TOK_EMPTY_LINE:
        done=true;
        // todo: read C code
        break;

      default:
        throw SyntaxError("key-value or separator expected");
    }
  }

  delete lex;
  delete in_stream;

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
    parseInput(buf,cb);
  } catch(SyntaxError &e) {
    free((void*)buf);
    throw e;
  }
  free((void*)buf);
}


