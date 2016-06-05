#include "vfs.h"

VFS::VFS() {

}

VFS::~VFS() {
  for(size_t i=0;i<_paths.len;i++)
    delete _paths[i];
}

void VFS::addPath(const alp::string &path) {
  if (path.len<1) return;
  if (path.ptr[path.len-1]!='/') {
    _paths.insert(new alp::string(path+"/"));
  } else {
    _paths.insert(new alp::string(path));
  }
}


#ifdef __linux

#include <sys/stat.h>
#include <unistd.h>

static bool _exists(const alp::string &fn) {
  struct stat buf;
  return stat(fn.ptr,&buf)==0;
}

alp::string VFS::locate(const alp::string &fn_in) {
  if (_exists(fn_in)) return fn_in;
  
  for(size_t i=0;i<_paths.len;i++) {
    alp::string tmp=*_paths[i]+fn_in;
    if (_exists(tmp)) return tmp;
  }

  return "";

}

#else 
#error "VFS::locate not implemented for this OS"
#endif
