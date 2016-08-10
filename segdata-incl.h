#include <assert.h>
#include <stdlib.h>

/** Data type to be used for segment boundaries and values.
  *
  * This is a vartype of integer and floating-points to cover all the possible
  * signatures of a target function and the corresponding domain/codomain.
  */
struct seg_data_t {
  enum kind_t {
    Integer,
    Double
  };  
  kind_t kind;
  union {
    double data_f;
    int64_t data_i;
  };

  seg_data_t() : kind(Integer), data_i(0) {
  }

  seg_data_t(double v) : kind(Double), data_f(v) {
  }
  seg_data_t(int v) : kind(Integer), data_i(v) {
  }
  seg_data_t(int64_t v) : kind(Integer), data_i(v) {
  }

  double operator=(double v) {
    kind=Double;
    data_f=v;
    return v;
  }

  int64_t operator=(int v) {
    kind=Integer;
    data_i=(v);
    return v;
  }
  int64_t operator=(int64_t v) {
    kind=Integer;
    data_i=(v);
    return v;
  }

  seg_data_t operator=(const seg_data_t &v) {
    kind=v.kind;
    switch(kind) {
      case Integer: data_i=v.data_i; break;
      case Double:  data_f=v.data_f; break;
    }
    return v;
  }
  bool operator==(const seg_data_t &v) const {
    switch(kind) {
      case Integer:
        switch(v.kind) {
          case Integer: return data_i==v.data_i;
          case Double: return data_i==v.data_f;
        }
      case Double:
        switch(v.kind) {
          case Integer: return data_f==v.data_i;
          case Double: return data_f==v.data_f;
        }
    }
    assert(0 && "Missing case in seg_data_t operator<=");
  }
  bool operator<=(const seg_data_t &v) const {
    switch(kind) {
      case Integer:
        switch(v.kind) {
          case Integer: return data_i<=v.data_i;
          case Double: return data_i<=v.data_f;
        }
      case Double:
        switch(v.kind) {
          case Integer: return data_f<=v.data_i;
          case Double: return data_f<=v.data_f;
        }
    }
    assert(0 && "Missing case in seg_data_t operator<=");
  }

  bool operator>=(const seg_data_t &v) const { return v<=*this; } 
  bool operator<(const seg_data_t &v) const { return !(v<=*this); } 
  bool operator>(const seg_data_t &v) const { return !(*this<=v); } 

  operator int() const {
    switch(kind) {
      default:
      case Integer: return (int)(int64_t)data_i;
      case Double:  return (int)data_f;
    }
  }
  operator int64_t() const {
    switch(kind) {
      default:
      case Integer: return (int64_t)data_i;
      case Double:  return (int64_t)data_f;
    }
  }
  operator double() const {
    switch(kind) {
      default:
      case Integer: return (double)data_i;
      case Double:  return data_f;
    }
  }
  
  /** Adds a conversion of b to our internal data.
    *
    * WARNING: this does not commute.
    */
  seg_data_t operator+(const seg_data_t &b) const {
    switch(kind) {
      default:
      case Integer: return seg_data_t(data_i+(int64_t)b);
      case Double:  return seg_data_t(data_f+(double)b);
    }
  }
  
  /** Subtracts a conversion of b to our internal data.
    *
    */
  seg_data_t operator-(const seg_data_t &b) const {
    switch(kind) {
      default:
      case Integer: return seg_data_t(data_i-(int64_t)b);
      case Double:  return seg_data_t(data_f-(double)b);
    }
  }

};
