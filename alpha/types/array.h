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

#ifndef _ALPHA_ARRAY_H
#define _ALPHA_ARRAY_H

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

namespace alp {

template<class C> struct array_t {
  C *ptr;
  size_t len;
  size_t ce_alloc;
  bool owned;
  
  
  array_t() : ptr(0), len(0), ce_alloc(0), owned(true) {
  
  }
  
  array_t(const array_t &a) {
    if (a.ptr) {
      ptr=(C*)malloc(a.ce_alloc*sizeof(C));
      len=a.len;
      ce_alloc=a.ce_alloc;
      if (len)
        memcpy(ptr,a.ptr,len*sizeof(C));
    } else {
      ptr=0;
      len=0;
      ce_alloc=0;
    }
    owned=true;
  }
  
  ~array_t() {
    if (owned&&ptr) {
      free((void*)ptr);
    }
  }
  
  size_t new_alloc_size(size_t newlen) {
    size_t newalloc=ce_alloc;
    
    if (newlen<1) return 0;
    
    if (!newalloc) newalloc=1;
    
    while (newalloc&&(newlen>newalloc)) newalloc<<=1;
    if (!newalloc) newalloc=newlen;
    
    while (newalloc&&(newalloc>>2>newlen)) newalloc>>=1;
    
    return newalloc;
  }
  
  void make_unique() {
    C *ptr1;
    if (owned) return;
    if (ptr) {
      ptr1=(C*)malloc(ce_alloc*sizeof(C));
      memcpy(ptr1,ptr,len*sizeof(C));
      ptr=ptr1;
      
    }
    owned=true;
  }
  
  void alloc(size_t count) {
    size_t newce_alloc=new_alloc_size(len+count);
    C *ptr1;
    
    if (newce_alloc<=ce_alloc) return;
    
    if (!owned) {
      ptr1=(C*)malloc(newce_alloc*sizeof(C));
      memcpy(ptr1,ptr,len*sizeof(C));
      ptr=ptr1;
    } else {
      ptr1=(C*)realloc((void*)ptr,newce_alloc*sizeof(C));
      if (ptr1) ptr=ptr1;
    }
    
    ce_alloc=newce_alloc;
    owned=true;
  }
  
  void setlen(size_t newlen) {
    size_t newce_alloc=new_alloc_size(newlen);
    C *ptr1;
    
    if (newlen==len) return;
    
    if (!owned) {
      if (!newce_alloc) {
        ptr=0;
      } else {
        ptr1=(C*)malloc(newce_alloc*sizeof(C));
        memcpy(ptr1,ptr,std::min(newlen,len)*sizeof(C));
        ptr=ptr1;
      }
    } else {
      if (!newce_alloc) {
        free((void*)ptr);
        ptr=0;
      } else if (newce_alloc!=ce_alloc) {
        ptr1=(C*)realloc((void*)ptr,newce_alloc*sizeof(C));
        if (ptr1) ptr=ptr1;
      }
    }
    len=newlen;
    ce_alloc=newce_alloc;
    owned=true;
  }
  
  void insert(C item, ssize_t before=-1) {
    setlen(len+1);
    
    make_unique();
    
    if ((before<0)||(before>=(ssize_t)len)) before=len-1;
    else {
      memmove(ptr+before+1,ptr+before,(len-before-1)*sizeof(C));
    }
    
    new(ptr+before) C(item);
    
  }
  
  void insert(C *items, size_t count, ssize_t before=-1) {
    size_t i;
    size_t len0=len;
    setlen(len+count);
    
    make_unique();
    
    if ((before<0)||(before>=len0)) before=len0;
    else {
      memmove(ptr+before+count,ptr+before,(len0-before)*sizeof(C));
    }
    memcpy(ptr+before,items,count*sizeof(C));
  }
  
  template<class BC> void insert_dup(
    C *items, size_t count, ssize_t before=-1) {
    
    size_t i;
    setlen(len+count);
    C *p;
    
    make_unique();
    
    if ((before<0)||(before>=len)) before=len;
    else {
      memmove(ptr+before+1,ptr+before,(len-before)*sizeof(C));
    }
    
    p=ptr+before;
    for(i=0;i<count;i++) *p++=new BC(*items[i]);
  }
  
  array_t dup() const {
    array_t a;
    a.len=len;
    a.ce_alloc=len;
    a.owned=true;
    if (len>0) {
      a.ptr=(C*)malloc(len*sizeof(C));
      memcpy(a.ptr,ptr,len*sizeof(C));
    } else {
      a.ptr=NULL;
    }
    return a;
  }
  
  void remove(size_t first, size_t count=1) {
    if (first>=len) return;
    if (first+count>len) count=len-first;
    if (count<1) return;
    
    make_unique();
    
    if (first+count<len) 
      memmove(ptr+first,ptr+first+count,(len-first-count)*sizeof(C));
    
    setlen(len-count);
  }
  
  void remove(C elem, bool remove_all=false) {
    size_t i=0;
    
    while(i<len)
      if (ptr[i]==elem) {
        remove(i);
        if (!remove_all) break;
      } else {
        i++;
      }
    
    
  }
  
  void remove_swapend(size_t idx) {
    if (idx>=len) return;
    make_unique();
    
    if (idx+1<len) 
      memmove(ptr+idx,ptr+len-1,sizeof(C));
    
    setlen(len-1);
  }
  
  C &operator[](int idx) { return *(ptr+idx); }
  C const &operator[](int idx) const { return *(ptr+idx); }
  
  void clear() {
    if (owned&&ptr)
      free((void*)ptr);
    
    ptr=0;
    ce_alloc=len=0;
  }
  
  void swap(int idx0, int idx1) {
    char buf[sizeof(C)];
    if (idx0<0) idx0=0;
    if (idx1<0) idx1=0;
    if (idx0>=len) idx0=len-1;
    if (idx1>=len) idx1=len-1;
    
    if (idx0==idx1) return;
    
    memcpy(buf,ptr+idx0,sizeof(C));
    memcpy(ptr+idx0,ptr+idx1,sizeof(C));
    memcpy(ptr+idx1,buf,sizeof(C));
  }
  
  void move(int idx0, int idx1) {
    char buf[sizeof(C)];
    
    if (idx0<0) idx0=0;
    if (idx1<0) idx1=0;
    if (idx0>=len) idx0=len-1;
    if (idx1>=len) idx1=len-1;
    
    if (idx0==idx1) return;
    
    if (idx0<idx1) {
      memcpy(buf,ptr+idx0,sizeof(C));
      memmove(ptr+idx0,ptr+idx0+1,sizeof(C)*(idx1-idx0));
      memcpy(ptr+idx1,buf,sizeof(C));
    } else if (idx0>idx1) {
      memcpy(buf,ptr+idx0,sizeof(C));
      memmove(ptr+idx1+1,ptr+idx1,sizeof(C)*(idx0-idx1));
      memcpy(ptr+idx1,buf,sizeof(C));
    }
  }
  
  void move_to_end(int idx) {
    char buf[sizeof(C)];
    
    if (idx>=len-1) return;
    memcpy(buf,ptr+idx,sizeof(C));
    memmove(ptr+idx,ptr+idx+1,sizeof(C)*(len-idx-1));
    memcpy(ptr+len-1,buf,sizeof(C));
    
  }
  
  
  bool contains(C elem) const {
    int i;
    for(i=0;i<len;i++) 
      if (memcmp(ptr+i,&elem,sizeof(C))==0) return true;
    return false;
  }
  
  int find(C elem) const {
    unsigned int i;
    for(i=0;i<len;i++) 
      if (memcmp(ptr+i,&elem,sizeof(C))==0) return i;
    return -1;
  }
  
  int find_eq(C elem) const {
    unsigned int i;
    for(i=0;i<len;i++) 
      if (ptr[i]==elem) return i;
    return -1;
  }
  
  typedef int cmp_t(const C &a, const C &b);
  void sort(cmp_t cmp) {
    if (len<2) return;
    make_unique();
    _qsort(cmp,0,len-1);
  }
  
  bool sorted(cmp_t cmp) const {
    if (len<2) return true;
    int i;
    for(i=1;i<len;i++) {
      if (cmp(ptr[i-1],ptr[i])>0) return false;
    }
    return true;
  
  }
  
  private:
    
    void _qsort(cmp_t cmp, ssize_t l, ssize_t r) {
      if (r<=l) return;
      ssize_t s=(l+r)/2;
      ssize_t i;
      
      {
        const C &bl=ptr[l], &br=ptr[r], &bs=ptr[s];
        
        if (cmp(bl,br)<0) {
          if (cmp(bs,bl)<0) s=l;
          else if (cmp(br,bs)<0) s=r;
        } else {
          if (cmp(bs,br)<0) s=r;
          else if (cmp(bl,bs)<0) s=l;
        }
      }
      swap(s,r);
      
      s=l;
      
      for(i=l;i<r;i++) if (cmp(ptr[i],ptr[r])<0) {
        swap(i,s);
        s++;
      }
      swap(s,r);
      
      _qsort(cmp,l,s-1);
      _qsort(cmp,s+1,r);
      
    }
  
};


template<class C> struct sorted_array_t {
  C *ptr;
  size_t len;
  size_t ce_alloc;
  bool owned;
  
  
  sorted_array_t() : ptr(0), len(0), ce_alloc(0), owned(true) {
  
  }
  
  sorted_array_t(const sorted_array_t &a) {
    if (a.ptr) {
      ptr=(C*)malloc(a.ce_alloc*sizeof(C));
      len=a.len;
      ce_alloc=a.ce_alloc;
      if (len)
        memcpy(ptr,a.ptr,len*sizeof(C));
    } else {
      ptr=0;
      len=0;
      ce_alloc=0;
    }
    owned=true;
  }
  
  ~sorted_array_t() {
    size_t i;
    if (owned&&ptr) {
      free((void*)ptr);
    }
  }
  
  size_t new_alloc_size(size_t newlen) {
    size_t newalloc=ce_alloc;
    
    if (newlen<1) return 0;
    
    if (!newalloc) newalloc=1;
    
    while (newalloc&&(newlen>newalloc)) newalloc<<=1;
    if (!newalloc) newalloc=newlen;
    
    while (newalloc&&(newalloc>>2>newlen)) newalloc>>=1;
    
    return newalloc;
  }
  
  void make_unique() {
    C *ptr1;
    size_t i;
    if (owned) return;
    if (ptr) {
      ptr1=(C*)malloc(ce_alloc*sizeof(C));
      memcpy(ptr1,ptr,len*sizeof(C));
      ptr=ptr1;
      
    }
    owned=true;
  }
  
  void setlen(size_t newlen) {
    size_t newce_alloc=new_alloc_size(newlen);
    C *ptr1;
    
    if (newlen==len) return;
    
    if (!owned) {
      if (!newce_alloc) {
        ptr=0;
      } else {
        ptr1=(C*)malloc(newce_alloc*sizeof(C));
        memcpy(ptr1,ptr,std::min(newlen,len)*sizeof(C));
        ptr=ptr1;
      }
    } else {
      if (!newce_alloc) {
        free((void*)ptr);
        ptr=0;
      } else if (newce_alloc!=ce_alloc) {
        ptr1=(C*)realloc((void*)ptr,newce_alloc*sizeof(C));
        if (ptr1) ptr=ptr1;
      }
    }
    len=newlen;
    ce_alloc=newce_alloc;
    owned=true;
  }
  
  void insert(C item) {
    setlen(len+1);
    
    make_unique();
    
    int l=0, r=len-2;
    for(l=0;l<len-1;l++) if (ptr[l]>item) { break; }
    
    while(l<r) {
      int c=(l+r)/2;
      if (ptr[c]==item) { l=r=c; break; }
      if (ptr[c]>item) { 
        if (c==l) { r=c; break; }
        r=c;
      } else {
        if (c==r) { l=c; break; }
        l=c+1;
      }
    }
    
    
    
    if ((l<0)||(l>=len)) l=len-1;
    else {
      memmove(ptr+l+1,ptr+l,(len-l-1)*sizeof(C));
    }
    
    new(ptr+l) C(item);
    
  }
  
  void remove(size_t first, size_t count=1) {
    size_t i;
    if (first>=len) return;
    if (first+count>len) count=len-first;
    if (count<1) return;
    
    make_unique();
    
    if (first+count<len) 
      memmove(ptr+first,ptr+first+count,(len-first-count)*sizeof(C));
    
    setlen(len-count);
  }
  
  C &operator[](int idx) { return *(ptr+idx); }
  C const &operator[](int idx) const { return *(ptr+idx); }
  
  void clear() {
    size_t i;
    
    if (owned&&ptr)
      free((void*)ptr);
    
    ptr=0;
    ce_alloc=len=0;
  }
  
  
  bool contains(C elem) const {
    int i;
    for(i=0;i<len;i++) 
      if (memcmp(ptr+i,&elem,sizeof(C))==0) return true;
  }
  
  int find(C elem) const {
    
    int l=0, r=len-1;
    
    while(l<r) {
      int c=(l+r)/2;
      if (ptr[c]==elem) { return c; }
      if (ptr[c]>elem) { 
        if (c==l) { return -1; }
        r=c-1;
      } else {
        if (c==r) { return -1; }
        l=c+1;
      }
    }
    
    return -1;
  }
  
  int find_ceil(C elem) const {
    int l=0, r=len-1;
    
    while(l<r) {
      int c=(l+r)/2;
      if (ptr[c]==elem) { return c; }
      if (ptr[c]>elem) { 
        if (c==l) { return -1; }
        r=c-1;
      } else {
        if (c==r) { return -1; }
        l=c+1;
      }
    }
    
    return l;
  
  }
  
  
};

} // namespace alp

#endif
