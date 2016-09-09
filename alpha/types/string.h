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

#ifndef _ALPHA_STRING_H
#define _ALPHA_STRING_H

#include "../utf.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

namespace alp {

char *strdup(const char *s);

struct substring {
  const char *ptr;
  size_t len;
  
  int idx_line;
  int idx_symbol;
  
  substring();
  substring(const char *str);
  substring(const char *str, size_t l);
  
  char *dup() const;
  
  int operator==(const substring &str) const;
  int operator==(const char *str) const;
  
  operator long() const {
    char buf[32];
    long res;
    char *endptr=0;
    
    if (len>31) {
      memcpy(buf,ptr,31);
      buf[31]=0;
    } else {
      memcpy(buf,ptr,len);
      buf[len]=0;
    }
    
    res=strtol(buf,&endptr,0);
    
    if (endptr<=buf) 
      return 0;
    
    return res;
  }
  operator unsigned long() const {
    char buf[32];
    unsigned long res;
    char *endptr=0;
    
    if (len>31) {
      memcpy(buf,ptr,31);
      buf[31]=0;
    } else {
      memcpy(buf,ptr,len);
      buf[len]=0;
    }
    
    res=strtoul(buf,&endptr,0);
    
    if (endptr<=buf) 
      return 0;
    
    return res;
  }
  
  operator int() const { return (int)(long)(*this); }
  operator unsigned int() const { return (unsigned int)(unsigned long)(*this); }
  
  operator double() const {
    char buf[32];
    double res;
    char *endptr=0;
    
    if (len>31) {
      memcpy(buf,ptr,31);
      buf[31]=0;
    } else {
      memcpy(buf,ptr,len);
      buf[len]=0;
    }
    
    res=strtod(buf,&endptr);
    
    if (endptr<=buf) 
      return 0;
    
    return res;
  }
  operator float() const {
    return (float)(double)(*this);
  }
  
};

template<class T> size_t strlen(const T *s) {
  size_t r;
  for(r=0;*s;r++,s++);
  return r;
}

template <class C> struct string_t {
  C *ptr;
  size_t len;
  size_t cc_alloc;
  bool owned;
  
  
  string_t() : ptr(0), len(0), cc_alloc(0), owned(true) { 
  }
  
  string_t(size_t cc) : ptr(0), len(0), cc_alloc(cc), owned(true) {
    if (cc) {
      ptr=(C*)malloc(cc*sizeof(C));
      ptr[0]=0;
    }
  }
  
  string_t(C *p, size_t l, size_t cca, bool o) : 
    ptr(p), len(l), cc_alloc(cca), owned(o) { }
  
  template<class S> string_t(const S *s, bool copy=true) {
    if (typeid(S)==typeid(C)) {
      len=strlen(s);
      if (copy) {
        cc_alloc=len+1;
        ptr=(C*)malloc(sizeof(C)*cc_alloc);
        memcpy(ptr,s,sizeof(C)*len);
        ptr[len]=0;
        owned=true;
      } else {
        cc_alloc=0;
        ptr=(C*)s;
        owned=false;
      }
    } else {
      ptr=utftranscode<S,C>(s,-1,&len);
      cc_alloc=len+1;
      owned=true;
    }
    
  }
  
  template<class S> string_t(
    const S *s, size_t l, bool copy=true) {
    
    if (typeid(S)==typeid(C)) {
      len=l;
      if (copy) {
        cc_alloc=len+1;
        ptr=(C*)malloc(sizeof(C)*cc_alloc);
        memcpy(ptr,s,sizeof(C)*len);
        ptr[len]=0;
        owned=true;
      } else {
        cc_alloc=0;
        ptr=(C*)s;
        owned=false;
      }
    } else {
      ptr=utftranscode<S,C>(s,l,&len);
      cc_alloc=len+1;
      owned=true;
    }
  }
  
  string_t(dchar s) {
    
    if (typeid(dchar)==typeid(C)) {
      len=1;
      cc_alloc=2;
      ptr=(C*)malloc(sizeof(C)*cc_alloc);
      ptr[0]=s;
      ptr[1]=0;
      owned=true;
    } else {
      ptr=utftranscode<dchar,C>(&s,1,&len);
      cc_alloc=len+1;
      owned=true;
    }
  }
  
  template<class S> string_t(const string_t<S> &s, bool copy=true) {
    if (typeid(S)==typeid(C)) {
      len=s.len;
      if (copy) {
        cc_alloc=len+1;
        ptr=(C*)malloc(sizeof(C)*cc_alloc);
        memcpy(ptr,s.ptr,sizeof(C)*len);
        ptr[len]=0;
        owned=true;
      } else {
        cc_alloc=0;
        ptr=(C*)s.ptr;
        owned=false;
      }
    } else {
      ptr=utftranscode<S,C>(s.ptr,s.len,&len);
      cc_alloc=len+1;
      owned=true;
    }
  }
  
  string_t(const string_t &s) {
    len=s.len;
    if (s.ptr) {
      cc_alloc=len+1;
      ptr=(C*)malloc(sizeof(C)*cc_alloc);
      memcpy(ptr,s.ptr,sizeof(C)*len);
      ptr[len]=0;
    } else {
      ptr=0;
      cc_alloc=0;
    }
    owned=true;
  }
  string_t(const string_t &s, bool copy) {
    len=s.len;
    if (copy) {
      cc_alloc=len+1;
      ptr=(C*)malloc(sizeof(C)*cc_alloc);
      memcpy(ptr,s.ptr,sizeof(C)*len);
      ptr[len]=0;
      owned=true;
    } else {
      cc_alloc=0;
      if (s.ptr) {
        ptr=(C*)s.ptr;
        owned=false;
      } else {
        ptr=0;
        owned=true;
      }
    }
  }
  
  string_t(const substring &s, bool copy=true) {
    if (typeid(C)==typeid(char)) {
      len=s.len;
      if (copy) {
        cc_alloc=len+1;
        ptr=(C*)malloc(sizeof(C)*cc_alloc);
        memcpy(ptr,s.ptr,sizeof(C)*len);
        ptr[len]=0;
        owned=true;
      } else {
        cc_alloc=0;
        ptr=(C*)s.ptr;
        owned=false;
      }
    } else {
      ptr=utftranscode<char,C>(s.ptr,s.len,&len);
      cc_alloc=len+1;
      owned=true;
    }
  }
  
  
  ~string_t() { 
    void *buf;
    if (ptr && owned) {
      buf=(void*)ptr;
      free(buf);
    }
  }
  
  static string_t Assign(C *s) {
    string_t r;
    size_t l=strlen(s);
    r.ptr=s;
    r.len=l;
    r.cc_alloc=0;
    r.owned=false;
    return r;
  }
  
  static string_t Assign(string_t &s) {
    string_t r;
    r.ptr=s.ptr;
    r.len=s.len;
    r.cc_alloc=0;
    r.owned=false;
    return r;
  }
  
  static string_t Format(const char *fmt, ...) {
    va_list args;
    
    va_start(args,fmt);
    string_t r=Format(fmt,args);
    va_end(args);
    
    return r;
    
  }
  static string_t Format(const char *fmt, va_list args_in) {
    int cc;
    char *str=0;
    va_list args,args2;
    string_t r;
    
    va_copy(args,args_in);
    va_copy(args2,args_in);
    
    cc=vsnprintf(str,0,fmt,args);
    if (cc) {
      str=(char*)malloc(cc+1);
      vsnprintf(str,cc+1,fmt,args2);
      if (typeid(C)==typeid(char)) {
        r.owned=true;
        r.ptr=str;
        r.cc_alloc=cc+1;
        r.len=cc;
      } else {
        r=string_t(str);
        free((void*)str);
      }
    }
    va_end(args);
    va_end(args2);
    
    return r;
  }
  
  template<class S> static string_t Limited(const S *ptr, size_t maxlen) {
    size_t i;
    for(i=0;i<maxlen;i++) if (!ptr[i]) { maxlen=i; break; }
    return string_t(ptr,maxlen);
  }

  template<class S> static string_t Unescape(const S *ptr, ssize_t len=-1) {
    if (len<0) len=utfcharlen(ptr);
    const S *p=ptr, *e=ptr+len;
    dchar c;
    string_t res;
    enum state_t {
      Base,
      EscapeBegin,
      ParseOctal,
      ParseHex,
    };
    state_t state=Base;
    int nDigits=0;
    dchar cParsed=0;
    

    while(p<e) {
      utfdecode(p,c);
      switch(state) {
        case Base: Base:
          if (c=='\\') state=EscapeBegin;
          else res+=c;
          break;
        case EscapeBegin:
          switch(c) {
            default:
              res+=c;
              state=Base;
              break;
            case 'a': res+='\a'; state=Base; break;
            case 'b': res+='\b'; state=Base; break;
            case 'f': res+='\f'; state=Base; break;
            case 'n': res+='\n'; state=Base; break;
            case 'r': res+='\r'; state=Base; break;
            case 't': res+='\t'; state=Base; break;
            case 'v': res+='\v'; state=Base; break;
            case 'x':
              nDigits=2;
              state=ParseHex;
              break;
            case 'u':
              nDigits=4;
              state=ParseHex;
              break;
            case 'U':
              nDigits=6;
              state=ParseHex;
              break;
            case '0': case '1': case '2': case '3': case '4': case '5':
            case '6': case '7':
              state=ParseOctal;
              nDigits=2;
              cParsed=c-'0';
              break;
          }
          break;
        #define ADDDIGIT(shamt,dig) { \
          cParsed=(cParsed<<shamt)|(dig); \
          if ((--nDigits<1) || (p>=e)) { \
            res+=cParsed; \
            state=Base; \
          } \
        }
        case ParseOctal:
          if ((c>='0') && (c<='7')) ADDDIGIT(3,c-'0')
          else {
            res+=cParsed;
            state=Base;
            goto Base;
          }
          break;
        case ParseHex:
          if ((c>='0') && (c<='7')) ADDDIGIT(4,c-'0')
          else if ((c>='a')&&(c<='f')) ADDDIGIT(4,c-'a'+10)
          else if ((c>='A')&&(c<='F')) ADDDIGIT(4,c-'A'+10)
          else {
            res+=cParsed;
            state=Base;
            goto Base;
          }
          break;
          #undef ADDDIGIT
      }
    }

    return res;
    

  }

  int codepoints() const {
    dchar c;
    const C *p=ptr, *e=ptr+len;
    int n=0;
    while((p!=NULL) && (p<e)) {
      utfdecode(p,c);
      n++;
    }
    return n;
  }

  int insertCodepoint(int index, dchar cp) {
    dchar c;
    C *p=ptr, *e=ptr+len, *p0=ptr;
    size_t offs;
    int cc;
    if ((ptr==NULL) or (len<1)) {
      assign(cp);
      return 0;
    }
    if (index<0) index=0;
    while((index>0) && (p!=NULL) && (p<e)) {
      utfdecode(p,c);
      index--;
    }
    offs=(size_t)p-(size_t)ptr;
    cc=utflen<C>(cp);
    resize(len+cc);
    memmove(ptr+offs,ptr+offs+cc,len-cc-offs);
    p=ptr+offs;
    utfencode(p,cc,cp);
    return offs;
  }

  int removeCodepoint(int index) {
    dchar c;
    C *p=ptr, *e=ptr+len, *p0=ptr;
    if (index<0) return 0;
    while((p!=NULL) && (p<e)) {
      utfdecode(p,c);
      if (index==0) {
        make_unique();
        memmove(p0,p,(size_t)e-(size_t)p);
        resize(len-((size_t)p-(size_t)p0));
        return (int)p-(int)p0;
      }
      index--;
      p0=p;
    }
    return 0;
  }
  
  void writef(const char *fmt, ...) {
    if (typeid(C)==typeid(char)) {
      int n;
      make_unique();
      
      va_list args;
      
      va_start(args,fmt);
      n=vsnprintf(ptr,cc_alloc,fmt,args);
      va_end(args);
      
      if ((n>0)&&(n<cc_alloc)) len=n;
      else if (cc_alloc>0) ptr[0]=0;
    }
    
  }
  
  void writef(const char *fmt, va_list args_in) {
    if (typeid(C)==typeid(char)) {
      int n;
      make_unique();
      
      va_list args;
      
      va_copy(args,args_in);
      n=vsnprintf(ptr,cc_alloc,fmt,args);
      va_end(args);
      if ((n>0)&&(n<cc_alloc)) len=n;
      else if (cc_alloc>0) ptr[0]=0;
    }
    
  }
  
  template<class S> void assign(const S *s) {
    C *ptr1;
    if (!owned) ptr=0;
    size_t l=strlen(s);
    
    if (typeid(S)==typeid(C)) {
      if (l+1>cc_alloc) {
        cc_alloc=l+1;
        ptr1=(C*)realloc((void*)ptr,cc_alloc*sizeof(C));
        if (ptr1) ptr=ptr1;
      }
      memcpy(ptr,s,l*sizeof(C));
      len=l;
      ptr[len]=0;
    } else {
      if (ptr) free((void*)ptr);
      ptr=utftranscode<S,C>(s,-1,&len);
      cc_alloc=len+1;
    }
    owned=true;
  }
  
  
  template<class S> void assign(const string_t<S> &s) {
    C *ptr1;
    if (!owned) ptr=0;
    if (typeid(S)==typeid(C)) {
      if (s.len+1>cc_alloc) {
        cc_alloc=s.len+1;
        ptr1=(C*)realloc((void*)ptr,cc_alloc*sizeof(C));
        if (ptr1) ptr=ptr1;
      }
      memcpy(ptr,s.ptr,s.len*sizeof(C));
      len=s.len;
      ptr[len]=0;
    } else {
      if (ptr) free((void*)ptr);
      ptr=utftranscode<S,C>(s.ptr,s.len,&len);
      cc_alloc=len+1;
    }
    
    owned=true;
  }
  
  void operator=(const string_t<char> &s) { assign(s); }
  void operator=(const string_t<wchar> &s) { assign(s); }
  void operator=(const string_t<dchar> &s) { assign(s); }
  void operator=(const char *s) { assign(s); }
  void operator=(const wchar *s) { assign(s); }
  void operator=(const dchar *s) { assign(s); }
  
  void operator=(const substring &s) {
    if (typeid(C)==typeid(char)) {
      resize(s.len);
      if (s.len>0) {
        memmove(ptr,s.ptr,s.len);
      }
      owned=true;
    } else {
      if (owned && (ptr!=NULL)) {
        free((void*)ptr);
        ptr=NULL;
        len=0;
        cc_alloc=0;
      }
      ptr=utftranscode<char,C>(s.ptr,s.len,&len);
      cc_alloc=len+1;
      owned=true;
    }
  }
  
  template<class S> void setRange(const S *p,const S *e) {
    string_t<S> s;
    s.ptr=(S*)p;
    s.len=(size_t)(e-p);
    s.owned=false;
    assign(s);
  }
  template<class S> void setRange(const S *p,size_t cc) {
    string_t<S> s;
    s.ptr=(S*)p;
    s.len=cc;
    s.owned=false;
    assign(s);
  }
  
  template<class S> bool operator==(const S *s) const {
    return utfeq(ptr,s,len);
  }
  template<class S> bool operator==(const string_t<S> &s) const {
    return utfeq(ptr,s.ptr,len,s.len);
  }
  
  template<class S> bool operator!=(const S *s) const {
    return !utfeq(ptr,s,len);
  }
  template<class S> bool operator!=(const string_t<S> &s) const {
    return !utfeq(ptr,s.ptr,len,s.len);
  }
  
  template<class S> bool operator<(const S *s) const {
    return utfeq(ptr,s,len)<0;
  }
  template<class S> bool operator<(const string_t<S> &s) const {
    return utfcmp(ptr,s.ptr,len,s.len)<0;
  }
  
  template<class S> bool operator>(const S *s) const {
    return utfeq(ptr,s,len)>0;
  }
  template<class S> bool operator>(const string_t<S> &s) const {
    return utfcmp(ptr,s.ptr,len,s.len)>0;
  }
  
  
  template<class S> bool operator<=(const S *s) const {
    return utfeq(ptr,s,len)<=0;
  }
  template<class S> bool operator<=(const string_t<S> &s) const {
    return utfcmp(ptr,s.ptr,len,s.len)<=0;
  }
  
  template<class S> bool operator>=(const S *s) const {
    return utfeq(ptr,s,len)>=0;
  }
  template<class S> bool operator>=(const string_t<S> &s) const {
    return utfcmp(ptr,s.ptr,len,s.len)>=0;
  }
  
  bool contains(const string_t<C> &substr, bool ignoreCase=false) {
    int i,j;
    if (substr.len>len) return false;
    if (substr.len<1) return true;
    
    if (ignoreCase) {
      for(i=0;i<=len-substr.len;i++) {
        for(j=0;j<substr.len;j++) 
          if (
            (ptr[i+j]!=substr.ptr[j]) && 
            (
              ((ptr[i+j]|0x20)!=(substr.ptr[j]|0x20)) ||
              ((substr.ptr[j]|0x20)<'a') ||
              ((substr.ptr[j]|0x20)>'z'))) break;
        if (j==substr.len) return true;
      }
    } else {
      for(i=0;i<=len-substr.len;i++) {
        for(j=0;j<substr.len;j++) 
          if (ptr[i+j]!=substr.ptr[j]) break;
        if (j==substr.len) return true;
      }
    }
    return false;
  }
  
  void clear() {
    if (!ptr) return;
    if (owned) {
      free((void*)ptr);
    }
    ptr=0;
    cc_alloc=0;
    len=0;
    owned=false;
  }
  
  /** \brief Returns a slice of our character array consisting of the character 
    * index interval [lower,upper). 
    */
  string_t sliced(int lower, int upper) const {
    string_t r;
    
    if (lower<0) lower=0;
    if (upper>=len) upper=len;
    
    if (upper>lower) {
      r.setRange(ptr+lower,ptr+upper);
    }
    return r;
  }
  
  void slice(int lower, int upper) {
    if (lower<0) lower=0;
    if (upper>=len) upper=len;
    
    if ((lower==0) && (upper==len)) return;
    make_unique();
    
    memmove(ptr,ptr+lower,(upper-lower)*sizeof(C));
    len=upper-lower;
    ptr[len]=0;
    
  }
  
  string_t operator+(dchar c) const {
    return operator+(string_t<dchar>(&c,(size_t)1));
  }
  
  string_t operator+(const C *s) const {
    string_t r;
    size_t l=strlen(s);
    
    r.len=len+l;
    r.cc_alloc=len+l+1;
    r.ptr=(C*)malloc(r.cc_alloc*sizeof(C));
    memcpy(r.ptr,ptr,len*sizeof(C));
    memcpy(r.ptr+len,s,l*sizeof(C));
    r.ptr[r.len]=0;
    return r;
  }
  string_t operator+(const string_t &s) const {
    string_t r;
    r.len=len+s.len;
    r.cc_alloc=len+s.len+1;
    r.ptr=(C*)malloc(r.cc_alloc*sizeof(C));
    memcpy(r.ptr,ptr,len*sizeof(C));
    memcpy(r.ptr+len,s.ptr,s.len*sizeof(C));
    r.ptr[r.len]=0;
    return r;
  }
  
  void operator+=(dchar c) {
    C buf[8];
    size_t cc_append;
    size_t i0=len;
    make_unique();
    
    cc_append=utfntranscode(buf,8,&c,1);
    resize(len+cc_append);
    
    memcpy(ptr+i0,buf,sizeof(C)*cc_append);
    ptr[len]=0;
  }
  
  template<class S> void append(const S *p, const S *e) {
    C *sc;
    C *ptr1;
    size_t slen=(size_t)(e-p);
    if (slen<1) return;
    make_unique();
    
    if (typeid(S)==typeid(C)) {
      if (len+slen+1>cc_alloc) {
        cc_alloc=len+slen+1;
        ptr1=(C*)realloc((void*)ptr,cc_alloc*sizeof(C));
        if (ptr1) ptr=ptr1;
      }
      memcpy(ptr+len,p,slen*sizeof(C));
      len+=slen;
      ptr[len]=0;
    } else {
      sc=utftranscode<S,C>(p,slen);
      *this+=sc;
      free((void*)sc);
    }
    owned=true;
  }
  
  template<class S> void append(const S *p, size_t slen) {
    C *sc;
    C *ptr1;
    if (slen<1) return;
    make_unique();
    
    if (typeid(S)==typeid(C)) {
      if (len+slen+1>cc_alloc) {
        cc_alloc=len+slen+1;
        ptr1=(C*)realloc((void*)ptr,cc_alloc*sizeof(C));
        if (ptr1) ptr=ptr1;
      }
      memcpy(ptr+len,p,slen*sizeof(C));
      len+=slen;
      ptr[len]=0;
    } else {
      sc=utftranscode<S,C>(p,slen);
      *this+=sc;
      free((void*)sc);
    }
    owned=true;
  }
  
  template<class S> void operator+=(const S *s) {
    size_t l=strlen(s);
    C *sc;
    C *ptr1;
    if (l<1) return;
    make_unique();
    
    if (typeid(S)==typeid(C)) {
      if (len+l+1>cc_alloc) {
        cc_alloc=len+l+1;
        ptr1=(C*)realloc((void*)ptr,cc_alloc*sizeof(C));
        if (ptr1) ptr=ptr1;
      }
      memcpy(ptr+len,s,l*sizeof(C));
      len+=l;
      ptr[len]=0;
    } else {
      sc=utftranscode<S,C>(s,l);
      *this+=sc;
      free((void*)sc);
    }
    owned=true;
  }
  
  template<class S> void operator+=(const string_t<S> &s) {
    C *sc;
    C *ptr1;
    if (s.len<1) return;
    make_unique();
    
    if (typeid(S)==typeid(C)) {
      if (len+s.len+1>cc_alloc) {
        cc_alloc=len+s.len+1;
        ptr1=(C*)realloc((void*)ptr,cc_alloc*sizeof(C));
        if (ptr1) ptr=ptr1;
      }
      memcpy(ptr+len,s.ptr,s.len*sizeof(C));
      len+=s.len;
      ptr[len]=0;
    } else {
      sc=utftranscode<S,C>(s.ptr,s.len);
      *this+=sc;
      free((void*)sc);
    }
    owned=true;
  }
  
  void operator*=(int n) {
    int len0;
    if (n<2) return;
    
    make_unique();
    len0=len;
    resize(len*n);
    
    while(--n) memcpy(ptr+n*len0,ptr,len0*sizeof(C));
    
  }
  
  void make_unique() {
    C *p;
    if (owned) return;
    p=(C*)malloc(len+1);
    memcpy(p,ptr,len*sizeof(C));
    p[len]=0;
    ptr=p;
    owned=true;
    cc_alloc=len+1;
  }
  
  void resize(size_t newlen, C chr=' ') {
    C *ptr1;
    if (newlen==len) return;
    
    if (newlen<len) {
      if (!owned) {
        if (newlen==0) {
          owned=true;
          ptr=0;
          len=0;
          cc_alloc=0;
          return;
        }
        make_unique();
      }
      
      len=newlen;
      ptr[len]=0;
    } else {
      if (newlen+1>cc_alloc) {
        cc_alloc=newlen+1;
        ptr1=(C*)realloc((void*)ptr,cc_alloc*sizeof(C));
        if (ptr1) ptr=ptr1;
      }
      while(len<newlen)
        ptr[len++]=chr;
      
      ptr[len]=0;
      
    }
    
  }
  
  void splice(size_t i0, size_t l) {
    if ((i0>=len)||(l<1)) return;
    if (i0+l>=len) {
      resize(i0);
    } else {
      make_unique();
      memmove(ptr+i0,ptr+i0+l,sizeof(C)*(len-i0-l));
      len-=l;
      ptr[len]=0;
    }
  }
  string_t spliced(size_t i0,size_t l) {
    string_t r(ptr,len,true);
    r.splice(i0,l);
    return r;
  }
  
  template<class S> void splice(size_t i0, size_t l, const S ins) {
    C buf[8];
    size_t cc_ins;
    int delta;
    int len0=len;
    
    if (i0>len) i0=len;
    if (i0+l>len) l=len-i0;
    
    cc_ins=utfntranscode(buf,8,&ins,1);
    
    delta=(int)cc_ins-(int)l;
    
    if (delta>0) {
      resize(len+delta);
    } else {
      make_unique();
      len+=delta;
    }
    memmove(ptr+i0+cc_ins,ptr+i0+l,sizeof(C)*(len0-i0-l));
    memcpy(ptr+i0,buf,sizeof(C)*cc_ins);
    ptr[len]=0;
  }
  template<class S> string_t spliced(size_t i0,size_t l, const S ins) {
    string_t r(ptr,len,true);
    r.splice(i0,l,ins);
    return r;
  }
  
  template<class S> void splice(size_t i0, size_t l, const S *ins) {
    size_t cc_ins;
    int delta;
    const C *inst;
    C *inst_free=0;
    int len0=len;
    
    if (i0>len) i0=len;
    if (i0+l>len) l=len-i0;
    
    if (typeid(S)==typeid(C)) {
      cc_ins=alp::strlen(ins);
      inst  =ins;
    } else {
      inst_free=utftranscode(ins,-1,&cc_ins);
      inst=inst_free;
    }
    
    delta=(int)cc_ins-(int)l;
    
    if (delta>0) {
      resize(len+delta);
    } else {
      make_unique();
      len+=delta;
    }
    memmove(ptr+i0+cc_ins,ptr+i0+l,sizeof(C)*(len0-i0-l));
    memcpy(ptr+i0,inst,sizeof(C)*cc_ins);
    ptr[len]=0;
    
    if (inst_free)
      free((void*)inst);
  }
  template<class S> string_t spliced(size_t i0,size_t l, const S *ins) {
    string_t r(ptr,len,true);
    r.splice(i0,l,ins);
    return r;
  }
  
  template<class S> void splice(size_t i0, size_t l, const string_t<S> &ins) {
    size_t cc_ins;
    int delta;
    const C *inst;
    C *inst_free=0;
    int len0=len;
    
    if (i0>len) i0=len;
    if (i0+l>len) l=len-i0;
    
    if (typeid(S)==typeid(C)) {
      cc_ins=ins.len;
      inst  =ins.ptr;
    } else {
      inst_free=utftranscode<S,C>(ins.ptr,ins.len,&cc_ins);
      inst=inst_free;
    }
    
    delta=(int)cc_ins-(int)l;
    
    if (delta>0) {
      resize(len+delta);
    } else {
      make_unique();
      len+=delta;
    }
    memmove(ptr+i0+cc_ins,ptr+i0+l,sizeof(C)*(len0-i0-l));
    memcpy(ptr+i0,inst,sizeof(C)*cc_ins);
    ptr[len]=0;
    
    if (inst_free)
      free((void*)inst);
  }
  template<class S> string_t spliced(
    size_t i0,size_t l, const string_t<S> &ins) {
    string_t r(ptr,len,true);
    r.splice(i0,l,ins);
    return r;
  }
  
  void normalize_path() {
    int i;
    
    make_unique();
    
    #ifdef __WIN32
    C delim='\\';
    #else
    C delim='/';
    #endif
    
    for(i=0;i<len;i++)
      if ((ptr[i]=='/')||(ptr[i]=='\\'))
        ptr[i]=delim;
  }
  
  string_t normalized_path() const {
    string_t c=*this;
    c.normalize_path;
    return c;
  }
  
  template<class S> void append_path(const S* path) {
    string_t<C> delim;
    #ifdef __WIN32
    C delim_char[2]={'\\',0};
    #else
    C delim_char[2]={'/',0};
    #endif
    if ((len>0) && (ptr[len-1]!='/') && (ptr[len-1]!='\\')) {
      delim.ptr=delim_char;
      delim.len=1;
      delim.owned=false;
      operator+=(delim);
    }
    operator+=<S>(path);
  }
  
  template<class S> void append_path(const string_t<S> &path) {
    string_t<C> delim;
    #ifdef __WIN32
    C delim_char[2]={'\\',0};
    #else
    C delim_char[2]={'/',0};
    #endif
    if ((len>0) && (ptr[len-1]!='/') && (ptr[len-1]!='\\')) {
      delim.ptr=delim_char;
      delim.len=1;
      delim.owned=false;
      operator+=(delim);
    }
    operator+=<S>(path);
  }
  
  string_t lower() const {
    int i;
    string_t r=*this;
    for(i=0;i<r.len;i++)
      if ((r.ptr[i]>='A')&&(r.ptr[i]<='Z')) r.ptr[i]|=0x20;
    
    return r;
  }
  string_t upper() const {
    int i;
    string_t r=*this;
    for(i=0;i<r.len;i++)
      if ((r.ptr[i]>='a')&&(r.ptr[i]<='z')) r.ptr[i]&=~0x20;
    
    return r;
  }
  
};

template<class L,class R> 
  inline string_t<R> operator+(const L *l, const string_t<R> &r) {
  string_t<R> rv=l;
  rv+=r;
  return rv;
}

typedef string_t<char> string;
typedef string_t<wchar> wstring;
typedef string_t<dchar> dstring;

} // namespace alp

#endif
