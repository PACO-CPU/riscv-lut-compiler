#include "arch-config.h"
#include "keyvalue.h"
#include "error.h"

#undef yyFlexLexer
#define yyFlexLexer BaseInputFlexLexer
#include <FlexLexer.h>
#include "input-lexer.h"

void arch_config_t::parse(const char *ptr, size_t cb, const char *name) {
  InputFlexLexer *lex=InputFlexLexer::New(ptr,cb,name);
  
  while(lex->yylex()!=0) {
    switch(lex->kind()) {
      case InputFlexLexer::TOK_IDENT: {
        alp::string name;
        KeyValue *kv=NULL;

        name=lex->strAttr();
        
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
        KVTEST("numSegments",Integer)
          numSegments=kv->val_num().data_i;
        KVTEST("wordSize",Integer)
          wordSize=kv->val_num().data_i;
        KVTEST("segmentBits",Integer)
          segmentBits=kv->val_num().data_i;
        KVTEST("selectorBits",Integer)
          selectorBits=kv->val_num().data_i;
        KVTEST("interpolationBits",Integer)
          interpolationBits=kv->val_num().data_i;
        KVTEST("plaInterconnects",Integer)
          plaInterconnects=kv->val_num().data_i;
        KVTEST("base_bits",Integer)
          base_bits=kv->val_num().data_i;
        KVTEST("incline_bits",Integer)
          incline_bits=kv->val_num().data_i;
        KVTEST("interpolationBits",Integer)
          plaInterconnects=kv->val_num().data_i;
        KVTEST("domainCutoffThreshold",Float)
          domainCutoffThreshold=kv->val_num().data_f;
        KVTEST("resolutionWarnThreshold",Float)
          resolutionWarnThreshold=kv->val_num().data_f;

        } else {
          throw SyntaxError("unknown arch key-value",lex);
        }

        #undef KVTEST

        break;
      }
      default:
        throw SyntaxError("key-value name expected",lex);
      
    }
  }

}

void arch_config_t::parseFile(const char *fn) {
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
    parse(buf,cb,fn);
  } catch(SyntaxError &e) {
    free((void*)buf);
    throw e;
  }
  free((void*)buf);
}

