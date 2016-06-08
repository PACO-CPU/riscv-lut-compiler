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

#include "logging.h"
#include "config.h"

#include <stdio.h>
#include <stdarg.h>

#ifdef __WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace alp {
  
  uint logmask=0;
  #if ALPHA_DEBUG
  uint logfilter=LOGF_INFO|LOGF_WARNING|LOGF_ERROR|LOGF_RESULT|LOGF_DEBUG|LOGF_STDOUT;
  #else
  uint logfilter=LOGF_INFO|LOGF_WARNING|LOGF_ERROR|LOGF_RESULT|LOGF_STDOUT;
  #endif
  uint loglevel=2;

  void logvf(const char *fmt, uint kind, va_list args) {
    if ((kind&LOGT_MASK)==LOGT_STDOUT) {
      if ( 
        ((1<<(kind&LOGT_MASK))&logfilter) 
        && (
          !(kind&LOGM_MASK) 
          ||(kind&logmask&LOGM_MASK))
        &&(LOGL_GET(kind)<=loglevel)) {
          vfprintf(stdout,fmt,args);
          fflush(stdout);
      
      }
    } else if (logf_stack::empty()) {
      if ( 
        ((1<<(kind&LOGT_MASK))&logfilter) 
        && (
          !(kind&LOGM_MASK) 
          ||(kind&logmask&LOGM_MASK))
        &&(LOGL_GET(kind)<=loglevel))
        switch(kind&LOGT_MASK) {
          #define PRINT(f,c) \
            fprintf(f,"\x1b[3" #c ";1m"); \
            vfprintf(f,fmt,args); \
            fprintf(f,"\x1b[30;0m"); \
            fflush(f); \
            break;
          default:
            PRINT(stdout,0)
          case LOGT_DEBUG:
            PRINT(stdout,5) // purple
          case LOGT_WARNING:
            PRINT(stderr,3) // yellow
          case LOGT_INFO:
            PRINT(stdout,6) // teal
          case LOGT_ERROR:
            PRINT(stderr,1) // red
          #undef PRINT
        }
    } else {
      logf_stack::log(fmt,kind,args);
    }
    
  }
  
  void logf(const char *fmt, uint kind, ...) {
    va_list args;
    va_start(args,kind);
    logvf(fmt,kind,args);
    va_end(args);
  }
  
  array_t<logf_stack> logf_stack::arr;
  
  bool speakr(const string_t<char> &msg) {
    #ifdef __WIN32
    HWND wnd;
    COPYDATASTRUCT cds;
    
    wnd = FindWindowA("Tfrm_speakr","speakR");
    if (!wnd) return false;
    
    cds.dwData = 0x3001;
    cds.cbData = msg.len;
    cds.lpData = (void*)msg.ptr;
    return SendMessageA(wnd,WM_COPYDATA,0,(LPARAM)&cds) == 2;
    
    #else
    int pid=fork();
    if (pid==-1) return false;
    if (pid==0) {
      execl("espeak","espeak",msg.ptr);
      exit(0);
    }
    return true;
    #endif
  }
  
  
  
} // namespace alp
