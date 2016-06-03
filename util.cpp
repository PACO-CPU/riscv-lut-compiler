#include <util.h>
#if defined(__linux)

#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

static bool delete_directory(const alp::string &fn, bool recurse) {
  DIR *dir;
  struct dirent *pent;
  alp::string fn_sub;
  struct stat st_sub;
  if (recurse) {
    dir=opendir(fn.ptr);
    if (dir==NULL) return false;

    while((pent=readdir(dir))!=NULL) {
      if ((strcmp(pent->d_name,".")==0)||(strcmp(pent->d_name,"..")==0))
        continue;
      fn_sub=fn+"/"+pent->d_name;
      if (stat(fn_sub.ptr,&st_sub)!=0) goto err;
      if (S_ISDIR(st_sub.st_mode) && !S_ISLNK(st_sub.st_mode)) {
        if (!delete_directory(fn_sub,true)) goto err;
      } else {
        if (unlink(fn_sub.ptr)!=0) goto err;
      }
    }
    closedir(dir);
    return rmdir(fn.ptr)==0;
    err:
      closedir(dir);
      return false;
  } else {
    return rmdir(fn.ptr)==0;
  }
}
TempDir::TempDir() {
  char tmp[]="/tmp/riscv-lut-compiler-XXXXXX";
  _path=mkdtemp(tmp);
  _path+="/";
}

TempDir::~TempDir() {
  delete_directory(_path,true);
}

#else
#error "TempDir not implemented for this OS"
#endif

