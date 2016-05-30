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

#endif
