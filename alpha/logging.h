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

#ifndef _ALPHA_LOGGING_H
#define _ALPHA_LOGGING_H

#include <pthread.h>

#include "types/base.h"
#include "types/array.h"
#include "types/string.h"


#include <stdarg.h>

namespace alp {
  enum : uint {
    LOGT_MASK   =0x0000000f,
    LOGT_INFO   =0x00000000,
    LOGT_WARNING=0x00000001,
    LOGT_ERROR  =0x00000002,
    LOGT_DEBUG  =0x00000003,
    LOGT_RESULT =0x00000004,
    LOGT_STDOUT =0x0000000f,
    
    LOGL_COUNT=0xf,
    LOGL_MASK =0x000000f0,
    LOGM_MASK =0x00ffff00,
    LOGF_SHOW =0x01000000,
    
    LOGF_INFO    =0x0001,
    LOGF_WARNING =0x0002,
    LOGF_ERROR   =0x0004,
    LOGF_DEBUG   =0x0008,
    LOGF_RESULT  =0x0010,
    LOGF_STDOUT  =0x8000,
  };
  
  extern uint logmask;
  extern uint logfilter;
  extern uint loglevel;
  
  #define LOGL(l) ( ((l)&0xf)<<4 )
  #define LOGL_GET(v) ({ uint _v=(v); (_v&LOGL_MASK)>>4;})
  #define LOGL_SET(v,l) ( ((v)&(~LOGL_MASK))|LOGL(l) )
  
  typedef void (*logf_t)(const char *,uint,va_list);
  
  struct logf_stack {
    bool exclusive;
    
    array_t<logf_t> funcs;
    
    logf_stack(logf_t f, bool ex) : exclusive(ex) {
      funcs.insert(f); 
    }
    
    private:
      static array_t<logf_stack> arr;
    
    public:
      static void remove(logf_t f) {
        int idx;
        uint i;
        for(i=0;i<arr.len;i++) {
          idx=arr[i].funcs.find(f);
          if (idx==-1) continue;
          if (arr[i].funcs.len==1) {
            arr.remove(i);
          } else {
            arr[i].funcs.remove(idx);
          }
          break;
        }
      }
      
      static void add(logf_t f, bool exclusive) {
        remove(f);
        
        if (exclusive||(arr.len<1)||(arr[arr.len-1].exclusive)) {
          arr.insert(logf_stack(f,exclusive));
        } else {
          arr[arr.len-1].funcs.insert(f);
        }
      }
      
      static bool empty() {
        return arr.len<1;
      }
      
      static void log(const char *fmt, uint kind, va_list args) {
        if (arr.len<1) return;
        uint i;
        for(i=0;i<arr[arr.len-1].funcs.len;i++)
          arr[arr.len-1].funcs[i](fmt,kind,args);
      }
    
  };
  
  void logvf(const char *fmt, uint kind, va_list args);
  void logf(const char *fmt, uint kind, ...);

  
  
  bool speakr(const string_t<char> &fmt);
  
  
  template<typename T> struct log_pbar {
    T min,max,next,cur;
    int idx;
    int kind;
    
    log_pbar(T min, T max, int kind=LOGT_INFO) : 
      min(min), max(max), next(min), idx(0), cur(min), kind(kind) {
    }
    
    void increment(T v) {
      static const char *image[41]={
        "0",".",".",".","1",".",".",".","2",".",".",".","3",".",".",".","4",
        ".",".",".","5",".",".",".","6",".",".",".","7",
        ".",".",".","8",".",".",".","9",".",".",".","10\n"
      };
      cur+=v;
      if ((idx!=1337) && (cur>=next)) {
        do {
          logf("%s",kind,image[idx++]);
          if (idx>=41) {
            idx=1337;
            break;
          } else {
            next=
              (T)(next+(double)(max-min)/(double)41);
          }
        } while(cur>next);
      }
    }
    
    void operator+=(T v) {
      increment(v);
    }
    
    void operator++(int a) { increment(1); }
    
    void finish() {
      increment(max);
    }
    void reset(T new_min, T new_max) {
      min=new_min; max=new_max;
      next=new_min;
      idx=0;
      cur=new_min;
    }
  };
  
  template<typename T> struct log_pbar_threadsafe {
    T min,max,next,cur;
    int idx;
    int kind;
    pthread_mutex_t m;
    
    log_pbar_threadsafe(T min, T max, int kind=LOGT_INFO) : 
      min(min), max(max), next(min), idx(0), cur(min), kind(kind),
      m(PTHREAD_MUTEX_INITIALIZER) {
    }
    
    void increment(T v) {
      static const char *image[41]={
        "0",".",".",".","1",".",".",".","2",".",".",".","3",".",".",".","4",
        ".",".",".","5",".",".",".","6",".",".",".","7",
        ".",".",".","8",".",".",".","9",".",".",".","10\n"
      };
      pthread_mutex_lock(&m);
      cur+=v;
      if ((idx!=1337) && (cur>=next)) {
        do {
          logf("%s",kind,image[idx++]);
          if (idx>=41) {
            idx=1337;
            break;
          } else {
            next=
              (T)(next+(double)(max-min)/(double)41);
          }
        } while(cur>next);
      }
      pthread_mutex_unlock(&m);
    
    }
    
    void operator+=(T v) {
      increment(v);
    }
    
    void operator++(int a) { increment(1); }
    
    void finish() {
      increment(max);
    }
    void reset(T new_min, T new_max) {
      pthread_mutex_lock(&m);
      min=new_min; max=new_max;
      next=new_min;
      idx=0;
      cur=new_min;
      pthread_mutex_unlock(&m);
    }
  };
};

#define LOG_DEBUG(flag,fmt) \
  if (flag) alp::logf(fmt,LOGT_DEBUG);
  
#define LOG_DEBUGF(flag,fmt,...) \
  if (flag) alp::logf(fmt,LOGT_DEBUG,__VA_ARGS__);


#endif
