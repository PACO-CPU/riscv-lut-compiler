#ifndef RISCV_LUT_COMPILER_UTIL_H
#define RISCV_LUT_COMPILER_UTIL_H

#include <iostream>
#include <istream>
#include <streambuf>
#include <string>
#include <alpha/alpha.h>
/** Helper struct for mapping char buffers onto c++ streams as expected by
  * flex-generated scanners.
  */
struct membuf_read_t : std::basic_streambuf<char>
{
    membuf_read_t(const char* begin, size_t cb) {
        this->setg((char*)begin, (char*)begin, (char*)begin+cb);
    }
};

/** Helper class for managing temporary directories.
  *
  * By simply creating and destroying objects of this class a new temporary
  * directory can be created and deleted (recursively).
  */
class TempDir {
  protected:
    alp::string _path;
  public:
    TempDir();
    ~TempDir();

    const alp::string &path() { return _path; }
};

#endif
