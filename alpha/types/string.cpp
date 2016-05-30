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

#include "string.h"

#include <string.h>
#include <stdlib.h>

namespace alp {

char *strdup(const char *s) {
  size_t l=strlen(s);
  char *r=(char*)malloc(l+1);
  memcpy(r,s,l+1);
  return r;
}

substring::substring() : ptr(0), len(0) {
}

substring::substring(const char *str) {
  ptr=str;
  len=strlen(ptr);
}

substring::substring(const char *str, size_t l) {
  ptr=str;
  len=l;
}

char *substring::dup() const {
  char *res;
  
  if ((len<1) || (ptr==NULL)) {
    res=(char*)malloc(1);
    res[0]=0;
  } else {
    res=(char*)malloc(len+1);
    memcpy(res,ptr,len);
    res[len]=0;
  }
  
  return res;
}

int substring::operator==(const substring &str) const {
  return (len==str.len) && (memcmp(ptr,str.ptr,len)==0);
}

int substring::operator==(const char *str) const {
  return (strncmp(ptr,str,len)==0) && (str[len]==0);
}

} // namespace alp
