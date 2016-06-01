#ifndef RISCV_LUT_COMPILER_ERROR_H
#define RISCV_LUT_COMPILER_ERROR_H
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
    SyntaxError(const alp::string &desc) {
      _msg="Syntax error: "+desc;
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
