
#include "dlib.h"
#include <stdlib.h>

#if defined(__linux)

#include <dlfcn.h>

struct dynamic_library_t {
  void *handle;
};


dynamic_library_t *dlib_open(const char *fn) {
  void *handle;
  
  handle=dlopen(fn,RTLD_LAZY);
  if (!handle) return NULL;
  dlerror();

  dynamic_library_t *res=new dynamic_library_t;
  res->handle=handle;
  return res;
}
void *dlib_lookup(dynamic_library_t *lib, const char *name) {
  if (!lib->handle) return NULL;
  return dlsym(lib->handle,name);
}
void dlib_close(dynamic_library_t *lib) {
  if (lib->handle!=NULL)
    dlclose(lib->handle);
  delete lib;
}



#else

#error "dynamic library loading not implemented for this OS"
#endif
