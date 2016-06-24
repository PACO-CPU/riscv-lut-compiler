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

#include "refcount.h"

namespace alp {

RefcountDestructException refcountDestructException;
RefcountDropException refcountDropException;

RCObject::RCObject() : _refcount(0) { }


RCObject::~RCObject() {
  #ifdef DEBUG
    if (_refcount!=0) {
      throw refcountDestructException;
    }
  #endif
}
void RCObject::grab() {
  _refcount++;
}

bool RCObject::drop() {
  if (_refcount<1)
    throw refcountDropException;
  if (_refcount==1) {
    _refcount=0;
    delete this;
    return true;
  } else {
    _refcount--;
  }
  return false;
}

int RCObject::refcount() { 
  return _refcount;
}




RCObjectv::RCObjectv() : _refcount(0) { }


RCObjectv::~RCObjectv() {
  #ifdef DEBUG
    if (_refcount!=0) {
      throw refcountDestructException;
    }
  #endif
}
void RCObjectv::grab() {
  _refcount++;
}

bool RCObjectv::drop() {
  if (_refcount<1)
    throw refcountDropException;
  if (_refcount==1) {
    _refcount=0;
    delete this;
    return true;
  } else {
    _refcount--;
  }
  return false;
}



int RCObjectv::refcount() { 
  return _refcount;
}

} // namespace alp
