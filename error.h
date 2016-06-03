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
  public:
    /** Constructor.
      * \param desc Description of the syntax error.
      */
    SyntaxError(const alp::string &desc, SourceLocationLexer *lex=NULL) {
      source_location_t loc;
      const char *pline, *p, *e;
      if (lex!=NULL) {
        loc=lex->loc();

        pline=lex->ptr();
        e=pline+lex->cb();
        pline+=loc.raw_offset-loc.cidx;
        for(p=pline+loc.cidx;p<e;p++) if ((*p=='\r')||(*p=='\n')) break;


        _msg=alp::string::Format(
          "\x1b[1m%s:%i:%i: \x1b[31;1merror: \x1b[30;0m%.*s\n%.*s\n",
          lex->unitName(),loc.lidx,loc.cidx,
          desc.len,desc.ptr,
          (size_t)p-(size_t)pline,pline);
        _msg.resize(_msg.len+loc.cidx+2,'-');
        _msg.ptr[_msg.len-2]='^';
        _msg.ptr[_msg.len-1]='\n';
      } else {
        _msg="Syntax error: "+desc;
      }
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
