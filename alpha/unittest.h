/*
  Copyright 2014 Peter Wagener
  
  This file is part of the alpha framework.
  
  The alpha framework is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  The alpha framework is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this code.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef _ALPHA_UNITTEST_H
#define _ALPHA_UNITTEST_H

#include "types/string.h"
#include <stdarg.h>

#ifndef ALPHA_UNITTESTS
#define ALPHA_UNITTESTS 1
#endif

namespace alp {

struct unittest_t {
  protected:
    int _nFailed;
    int _nSucceeded;
    void _finalize();
    const char *_name;
  public:
    unittest_t(const char *name) : _nFailed(0), _nSucceeded(0), _name(name) {

    }
    void Assert(bool cond, const string &msg);
    void Assertf(bool cond, const char *fmt, ...);
};

#define unittest_concat_id(a,b) a##b
#define unittest_stringify(a) #a
#define unittest_with_id(id,ln,...) \
namespace { \
struct unittest_concat_id(unittest_,id) : public alp::unittest_t { \
  unittest_concat_id(unittest_,id)() : \
    alp::unittest_t( \
      "unittest # " unittest_stringify(id) \
      " (" __FILE__ ":" unittest_stringify(ln) ")") { \
    __VA_ARGS__ \
    _finalize(); \
  } \
} unittest_concat_id(_unittest_,id); \
}

#define unittest(...) unittest_with_id(__COUNTER__,__LINE__,__VA_ARGS__)
} // namespace alp
#endif
