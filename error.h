#ifndef RISCV_LUT_COMPILER_ERROR_H
#define RISCV_LUT_COMPILER_ERROR_H
#include "lexer-common.h"
#include <exception>
#include <alpha/alpha.h>

/** Exception thrown when an error occurs with a file.
  *
  * This error can be a missing file, permission error or general read/write
  * errors.
  *
  * \todo differentiate different errors
  */
class FileIOException : public std::exception {
  protected:
    alp::string _msg;
  public:
    /** Constructor.
      * \param fn Name of the file causing problems.
      */
    FileIOException(const alp::string &fn) {
      _msg="File i/o error: "+fn;
    }

    virtual const char *what() const noexcept {
      return _msg.ptr;
    }
};

/** Exception thrown when a syntax error occurs during the parsing of a file.
  */
class SyntaxError : public std::exception {
  protected:
    alp::string _msg;
    
    static alp::string _FormatMessage(
      const alp::string &desc, SourceLocationLexer *lex, 
      source_location_t &loc, int color=1, const char *prefix="error") {

      alp::string msg;
      const char *pline, *p, *e;

      if (lex!=NULL) {
        pline=lex->ptr();
        e=pline+lex->cb();
        pline+=loc.raw_offset-loc.cidx;
        for(p=pline+loc.cidx;p<e;p++) if ((*p=='\r')||(*p=='\n')) break;


        msg=alp::string::Format(
          "\x1b[1m%s:%i:%i: \x1b[3%i;1m%s: \x1b[30;0m%.*s\n%.*s\n",
          lex->unitName(),loc.lidx,loc.cidx,
          color,prefix,
          desc.len,desc.ptr,
          (size_t)p-(size_t)pline,pline);
        msg.resize(msg.len+loc.cidx+2,'-');
        msg.ptr[msg.len-2]='^';
        msg.ptr[msg.len-1]='\n';
      } else {
        msg=alp::string::Format(
          "\x1b[3%i;1mSyntax %s\x1b[30;0m: %.*s\n",
          color,prefix,desc.len,desc.ptr);
      }
      return msg;
    }
  public:
    /** Constructor.
      * \param desc Description of the syntax error.
      * \param lex Optional lexer to use for building the error message.
      */
    SyntaxError(const alp::string &desc, SourceLocationLexer *lex=NULL) {
      _msg=FormatMessage(desc,lex);
    }
    /** Constructor.
      * \param desc Description of the syntax error.
      * \param lex Lexer to use for building the error message.
      * \param loc_in Source location to refer to
      */
    SyntaxError(
      const alp::string &desc, 
      SourceLocationLexer *lex, const source_location_t &loc_in) {
      source_location_t loc=loc_in;
      if (loc.raw_offset>=lex->cb()) _msg=FormatMessage(desc);
      else _msg=FormatMessage(desc,lex,loc);
    }

    virtual const char *what() const noexcept {
      return _msg.ptr;
    }

    static alp::string FormatMessage(
      const alp::string &desc, SourceLocationLexer *lex=NULL, int color=1, 
      const char *prefix="error") {
      source_location_t loc;
      if (lex!=NULL) loc=lex->loc();
      return _FormatMessage(desc,lex,loc,color,prefix);
    }
    static alp::string FormatMessage(
      const alp::string &desc, SourceLocationLexer *lex, 
      const source_location_t &loc_in, int color=1, 
      const char *prefix="error") {
      source_location_t loc=loc_in;
      if (loc.raw_offset>=lex->cb()) return FormatMessage(desc,NULL,loc);
      else return FormatMessage(desc,lex,loc);
    }
};

#define SyntaxWarning(desc,lex) { \
  alp::string msg=SyntaxError::FormatMessage(desc,lex,3,"warning"); \
  alp::logf("%.*s",alp::LOGT_WARNING,msg.len,msg.ptr); \
}


/** Exception thrown when an error occurs during the execution of a job
  */
class RuntimeError : public std::exception {
  protected:
    alp::string _msg;
  public:
    /** Constructor.
      * \param desc Description of the error..
      */
    RuntimeError(const alp::string &desc) :
      _msg(desc) {
    
    }

    virtual const char *what() const noexcept {
      return _msg.ptr;
    }
};



/** Exception thrown when an error occurs during the parsing of a command line.
  */
class CommandLineError : public std::exception {
  public:
    enum kind_t {
      UnknownSwitch,
      StrayArgument,
      MissingArgument,
      Semantics
    }; 
  protected:
    alp::string _msg;
    kind_t _kind;
  public:
    /** Constructor.
      * \param desc Description of the syntax error.
      */
    CommandLineError(kind_t kind, const alp::string &desc) {
      _kind=kind;
      switch(kind) {
        case UnknownSwitch:   _msg="Unknown switch: "+desc; break;
        case StrayArgument:   _msg="Stray argument: "+desc; break;
        case MissingArgument: _msg="Missing argument for "+desc; break;
        case Semantics      : _msg=desc; break;
      }
    }

    kind_t kind() const { return _kind; }

    virtual const char *what() const noexcept {
      return _msg.ptr;
    }
};

#endif
