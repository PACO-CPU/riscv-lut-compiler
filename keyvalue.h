#ifndef RISCV_LUT_COMPILER_KEYVALUE_H
#define RISCV_LUT_COMPILER_KEYVALUE_H
#include "segment.h"
#include <alpha/alpha.h>

/** Represents a single key-value as read by input files and stored in a
  * LookupTable instance.
  *
  * Instances of this class are considered immutable, thus neither key nor
  * value can be changed outside of the constructor.
  */
class KeyValue {
  public:
    enum kind_t {
      String =0,
      Integer,
      Float,
    };
  protected:
    alp::string _name;
    kind_t _kind;
    alp::string _val_str;
    seg_data_t _val_num;

  public:
    
    
    KeyValue(const alp::string &name, const alp::string &val) :
      _name(name), _kind(String), _val_str(val) {

    }
    
    KeyValue(const alp::string &name, const seg_data_t &val) :
      _name(name), _kind(val.kind==seg_data_t::Double ? Float : Integer), 
      _val_num(val) {

    }

    const alp::string &name() { return _name; }
    kind_t kind() { return _kind; }
    const alp::string &val_str() { return _val_str; }
    const seg_data_t  &val_num() { return _val_num; }
};


#endif
