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

#ifndef _ALPHA_UTF_H
#define _ALPHA_UTF_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "types/base.h"

#include <typeinfo>


namespace alp {


extern dchar __utf_fallback_map32[256]; // todo: create a default charmap

template <class T> int utflen(dchar cp) {
  return 0;
}

template<char> int utflen(dchar cp) {
  if (cp & 0xffffff80) { // 8..31 bits - 2 to 6 bytes
    if (cp & 0xfffe0000) { // 17..31 bits - 4 to 6 bytes
      if (cp & 0xffc00000) { // 22..31 bits - 5 or 6 bytes
        if (cp & 0xf8000000) { // 27..31 bits - 6 bytes
          return 6;
        } else { // 22..26 bits - 5 bytes
          return 5;
        }
      } else { // 16..21 bits - 4 bytes
        return 4;
      }
    } else { // 8..16 bits - 2 or 3 bytes
      if (cp & 0x1fffff000) { // 12..16 bits - 3 bytes
        return 3;
      } else { // 8..11 bits - 2 bytes
        return 2;
      }
    }
  } else { // 0..7 bits - 1 byte
    return 1;
  }
}

template<wchar> int utflen(dchar cp) {
  if (cp<0x10000) {
    return 1;
  } else {
    return 2;
  }
}

template<dchar> int utflen(dchar cp) {
  return 1;
}

inline int utfdecode(const char *&p, dchar &cp) {
  
  if (*p&0x80) { // multibyte or extended ASCII
    
    if        ((*p&0b11100000)==0b11000000) {
      if (!p[1]) goto invalid_sequence;
      cp=(*p & 0b00011111) <<  6 | (p[1] & 0x3f);
      p+=2;
      return 2;
    } else if ((*p&0b11110000)==0b11100000) {
      if (!p[1]||!p[2]) goto invalid_sequence;
      cp=(*p & 0b00001111) << 12 | (p[1] & 0x3f) <<  6 | (p[2] & 0x3f);
      p+=3;
      return 3;
    } else if ((*p&0b11111000)==0b11110000) {
      if (!p[1]||!p[2]||!p[3]) goto invalid_sequence;
      cp=(*p & 0b00000111) << 18 | (p[1] & 0x3f) << 12 | (p[2] & 0x3f) <<  6 | (p[3] & 0x3f);
      p+=4;
      return 4;
    } else if ((*p&0b11111100)==0b11111000) {
      if (!p[1]||!p[2]||!p[3]||!p[4]) goto invalid_sequence;
      cp=(*p & 0b00000011) << 24 | (p[1] & 0x3f) << 18 | (p[2] & 0x3f) << 12 | (p[3] & 0x3f) <<  6 | (p[4] & 0x3f);
      p+=5;
      return 5;
    } else if ((*p&0b11111110)==0b11111100) {
      if (!p[1]||!p[2]||!p[3]||!p[4]||!p[5]) goto invalid_sequence;
      cp=(*p & 0b00000001) << 30 | (p[1] & 0x3f) << 24 | (p[2] & 0x3f) << 18 | (p[3] & 0x3f) << 12 | (p[4] & 0x3f) << 6 | (p[5] & 0x3f);
      p+=6;
      return 6;
    } else {
      goto invalid_sequence;
    }
    
  } else {
    cp=*p++;
    return 1;
  }
  
  invalid_sequence:
  cp=__utf_fallback_map32[(unsigned char)*p++];
  return 1;
}
inline int utfdecode(char *&p, dchar &cp) {
  
  if (*p&0x80) { // multibyte or extended ASCII
    
    if        ((*p&0b11100000)==0b11000000) {
      if (!p[1]) goto invalid_sequence;
      cp=(*p & 0b00011111) <<  6 | (p[1] & 0x3f);
      p+=2;
      return 2;
    } else if ((*p&0b11110000)==0b11100000) {
      if (!p[1]||!p[2]) goto invalid_sequence;
      cp=(*p & 0b00001111) << 12 | (p[1] & 0x3f) <<  6 | (p[2] & 0x3f);
      p+=3;
      return 3;
    } else if ((*p&0b11111000)==0b11110000) {
      if (!p[1]||!p[2]||!p[3]) goto invalid_sequence;
      cp=(*p & 0b00000111) << 18 | (p[1] & 0x3f) << 12 | (p[2] & 0x3f) <<  6 | (p[3] & 0x3f);
      p+=4;
      return 4;
    } else if ((*p&0b11111100)==0b11111000) {
      if (!p[1]||!p[2]||!p[3]||!p[4]) goto invalid_sequence;
      cp=(*p & 0b00000011) << 24 | (p[1] & 0x3f) << 18 | (p[2] & 0x3f) << 12 | (p[3] & 0x3f) <<  6 | (p[4] & 0x3f);
      p+=5;
      return 5;
    } else if ((*p&0b11111110)==0b11111100) {
      if (!p[1]||!p[2]||!p[3]||!p[4]||!p[5]) goto invalid_sequence;
      cp=(*p & 0b00000001) << 30 | (p[1] & 0x3f) << 24 | (p[2] & 0x3f) << 18 | (p[3] & 0x3f) << 12 | (p[4] & 0x3f) << 6 | (p[5] & 0x3f);
      p+=6;
      return 6;
    } else {
      goto invalid_sequence;
    }
    
  } else {
    cp=*p++;
    return 1;
  }
  
  invalid_sequence:
  cp=__utf_fallback_map32[(unsigned char)*p++];
  return 1;
}
inline int utfdecode(const wchar *&p, dchar &cp) {
  wchar f;
  
  if ((*p>0xd7ff) && (*p<0xe000)) { // multichar UTF-16
    if (((f=p[1])<0xdc00) || (f>0xdfff)) goto invalid_sequence;
    
    cp=0x10000+( ((*p&0x3ff)<<10) | (f&0x3ff) );
    p+=2;
    return 2;
  } else {
    cp=*p++;
    return 1;
  }
  
  invalid_sequence:
  // some encoders just use the lower 16 bits of a codepoint instead of properly
  // encoding UTF-16. Assume we have one of those on our hands if we encounter
  // an invalid sequence.
  cp=*p++;
  return 1;
}

inline int utfdecode(wchar *&p, dchar &cp) {
  wchar f;
  
  if ((*p>0xd7ff) && (*p<0xe000)) { // multichar UTF-16
    if (((f=p[1])<0xdc00) || (f>0xdfff)) goto invalid_sequence;
    
    cp=0x10000+( ((*p&0x3ff)<<10) | (f&0x3ff) );
    p+=2;
    return 2;
  } else {
    cp=*p++;
    return 1;
  }
  
  invalid_sequence:
  // some encoders just use the lower 16 bits of a codepoint instead of properly
  // encoding UTF-16. Assume we have one of those on our hands if we encounter
  // an invalid sequence.
  cp=*p++;
  return 1;
}

inline int utfdecode(const dchar *&p, dchar &cp) {
  cp=*p++;
  return 1;
}

inline int utfdecode(dchar *&p, dchar &cp) {
  cp=*p++;
  return 1;
}

inline int utfencode(char *&p, size_t cc, dchar cp) {
  
  
  if (cp & 0xffffff80) { // 8..31 bits - 2 to 6 bytes
    if (cp & 0xfffe0000) { // 17..31 bits - 4 to 6 bytes
      if (cp & 0xffc00000) { // 22..31 bits - 5 or 6 bytes
        if (cp & 0xf8000000) { // 27..31 bits - 6 bytes
          if (cp & 0x80000000) goto invalid_sequence;
          if (cc<6) return -6; 
          *p++ = (cp & 0x40000000) ? 0xfd : 0xfc;
          *p++ = 0x80 | (cp & 0x3f000000) >> 24;
          *p++ = 0x80 | (cp & 0x00fc0000) >> 18;
          *p++ = 0x80 | (cp & 0x0003f000) >> 12;
          *p++ = 0x80 | (cp & 0x00000fc0) >>  6;
          *p++ = 0x80 | (cp & 0x0000003f)      ;
          return 6;
        } else { // 22..26 bits - 5 bytes
          if (cc<5) return -5; 
          *p++ = 0xf8 | (cp & 0x03000000) >> 24;
          *p++ = 0x80 | (cp & 0x00fc0000) >> 18;
          *p++ = 0x80 | (cp & 0x0003f000) >> 12;
          *p++ = 0x80 | (cp & 0x00000fc0) >>  6;
          *p++ = 0x80 | (cp & 0x0000003f)      ;
          return 5;
        }
      } else { // 16..21 bits - 4 bytes
        if (cc<4) return -4;
        *p++ = 0xf0 | (cp & 0x001c0000) >> 18;
        *p++ = 0x80 | (cp & 0x0003f000) >> 12;
        *p++ = 0x80 | (cp & 0x00000fc0) >>  6;
        *p++ = 0x80 | (cp & 0x0000003f)      ;
        return 4;
      }
    } else { // 8..16 bits - 2 or 3 bytes
      if (cp & 0x1fffff000) { // 12..16 bits - 3 bytes
        if (cc<3) return -3;
        *p++ = 0xe0 | (cp & 0x0000f000) >> 12;
        *p++ = 0x80 | (cp & 0x00000fc0) >>  6;
        *p++ = 0x80 | (cp & 0x0000003f)      ;
        return 3;
      } else { // 8..11 bits - 2 bytes
        if (cc<2) return -2;
        *p++ = 0xc0 | (cp & 0x000007c0) >>  6;
        *p++ = 0x80 | (cp & 0x0000003f)      ;
        return 2;
      }
    }
  } else { // 0..7 bits - 1 byte
    if (cc<1) return -1; 
    *p++ = (char)cp;
    return 1;
  }
  
  invalid_sequence:
  return 0;
}

inline int utfencode(wchar *&p, size_t cc, dchar cp) {
  
  if ((cp==0xd800)||(cp==0xdfff)) return 1;
  
  if (cp<0x10000) {
    *p++=(wchar)cp;
    return 1;
  } else {
    if (cc<2) return -2;
    cp-=0x10000;
    *p++=(wchar)(0xd800|((cp>>10)%0x3ff));
    *p++=(wchar)(0xdc00|((cp    )%0x3ff));
    
    return 2;
  }
  /*
  
  if ((cp&0xffef0000)||(cp&0xd800)) return 1;
  
  if (cp & 0xff0000) {
    
    if (cc<2) return -2; 
    *p++=0xd800 | ((cp>>10)&0x3ff);
    *p++=0xdc00 | ((cp    )&0x3ff);
    return 2;
  } else {
    
    *p++=(wchar)cp;
    return 1;
  }
  */
}

inline int utfencode(dchar *&p, size_t cc, dchar cp) {
  if (cc<1) return -1;
  *p++=cp;
  return 1;
}

template<class S, class T> inline int utfntranscode(
  T *target, size_t cc_buf, const S *source, ssize_t cc_in=-1, size_t *pcs=0) {
  dchar cp;
  int r;
  int cc=0;
  int cs=0;
  
  T buf[8];
  T *pbuf;
  
  if (cc_buf==1) {
    *target=0;
    return 0;
  }
  if (!target || (cc_buf==0)) {
    
    if (cc_in>-1) {
      while(cc_in) {
        cc_in-=utfdecode(source,cp);
        pbuf=buf;
        r=utfencode(pbuf,8,cp);
        cc+=r;
        cs++;
      }
    } else {
      while(*source&&utfdecode(source,cp)) {
        pbuf=buf;
        r=utfencode(pbuf,8,cp);
        cc+=r;
        cs++;
      }
    }
    if (pcs) *pcs=cs;
    return cc;
  }
  
  cc_buf--;
  
  if (cc_in>-1) {
    while(cc_in) {
      cc_in-=utfdecode(source,cp);
      r=utfencode(target,cc_buf,cp);
      if (r<1) break;
      cc+=r;
      cs++;
    }
  } else {
    while(*source&&utfdecode(source,cp)) {
      r=utfencode(target,cc_buf,cp);
      if (r<1) break;
      cc+=r;
      cs++;
    }
  
  }
  *target=0;
  if (pcs) *pcs=cs;
  return cc;
}

template<class S, class T> inline int utfntranscode_stream(
  T *target, size_t cc_buf, const S *source, ssize_t cc_in=-1) {
  dchar cp;
  int r;
  int cc=0;
  
  T buf[8];
  T *pbuf;
  
  
  if (cc_buf==0) {
    
    if (cc_in>-1) {
      while(cc_in) {
        cc_in-=utfdecode(source,cp);
        pbuf=buf;
        r=utfencode(pbuf,8,cp);
        cc+=r;
      }
    } else {
      while(*source&&utfdecode(source,cp)) {
        pbuf=buf;
        r=utfencode(pbuf,8,cp);
        cc+=r;
      }
    }
    return cc;
  }
  
  
  if (cc_in>-1) {
    while(cc_in) {
      cc_in-=utfdecode(source,cp);
      r=utfencode(target,cc_buf,cp);
      if (r<1) break;
      cc+=r;
    }
  } else {
    while(*source&&utfdecode(source,cp)) {
      r=utfencode(target,cc_buf,cp);
      if (r<1) break;
      cc+=r;
    }
  
  }
  return cc;
}

template<class S, class T> inline T *utftranscode(
  const S *source, ssize_t cc_in=-1, size_t *pcc_out=0, size_t *pcs=0) {
  
  int cc_target;
  T *res;
  
  cc_target=utfntranscode<S,T>(0,0,source,cc_in,pcs);
  
  res=(T*)malloc(sizeof(T)*(cc_target+1));
  
  utfntranscode<S,T>(res,cc_target+1,source,cc_in);
  
  res[cc_target]=0;
  
  if (pcc_out)
    *pcc_out=cc_target;
  
  return res;

}

template<class A, class B> inline int utfcmp(const A *a, const B *b, ssize_t cca=-1, ssize_t ccb=-1) {
  const A *ea;
  const B *eb;
  dchar cpa, cpb;
  if (cca<0) cca=strlen(a);
  if (ccb<0) ccb=strlen(b);
  
  ea=a+cca;
  eb=b+ccb;
  
  while((a<ea) && (b<eb)) {
    utfdecode(a,cpa);
    utfdecode(b,cpb);
    if (cpa==cpb) continue;
    if (cpa<cpb) return -1;
    return 1;
  }
  
  if (a<ea) return -1;
  if (b<eb) return 1;
  return 0;
}

template<class S> inline size_t utfcharlen(const S *ptr) {
  size_t s;
  for(s=0;*ptr;ptr++,s++);
  return s;
}

template<class A, class B> inline bool utfeq(const A *a, const B *b, ssize_t cca=-1, ssize_t ccb=-1) {;
  const A *ea;
  const B *eb;
  dchar cpa, cpb;
  if (cca<0) cca=utfcharlen(a);
  if (ccb<0) ccb=utfcharlen(b);
  
  if (typeid(A)==typeid(B)) {
    return (cca==ccb) && (memcmp(a,b,cca*sizeof(A))==0);
  } else {
    
    ea=a+cca;
    eb=b+ccb;
    
    while((a<ea) && (b<eb)) {
      utfdecode(a,cpa);
      utfdecode(b,cpb);
      if (cpa!=cpb) return false;
    }
    return true;
  }
}



} // namespace alp

#endif
