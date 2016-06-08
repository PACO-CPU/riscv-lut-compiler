#include "unittest.h"
#include "logging.h"

namespace alp {

void unittest_t::_finalize() {
  if (_nFailed) {
    logf(
      "unittest %s: %i / %i asserts failed\n",LOGT_ERROR,
      _name,_nFailed,_nFailed+_nSucceeded);
  }
}

void unittest_t::Assert(bool cond, const string &msg) {
  if (cond) {
    _nSucceeded++;
  } else {
    logf(msg.ptr,LOGT_ERROR);
    _nFailed++;
  }
}

void unittest_t::Assertf(bool cond, const char *fmt,...) {
  if (cond) {
    _nSucceeded++;
  } else {
    va_list args;
    va_start(args,fmt);
    logvf(fmt,LOGT_ERROR,args);
    va_end(args);
    _nFailed++;
  }
}

} // namespace alp
