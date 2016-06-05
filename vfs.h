#ifndef RISCV_LUT_COMPILER_VFS_H
#define RISCV_LUT_COMPILER_VFS_H

#include <alpha/alpha.h>

/** Virtual file system class used for resolving relative file names with
  * multiple base directories.
  */
class VFS {
  protected:
    alp::array_t<alp::string*> _paths;

  public:
    
    VFS();
    
    /** Copy constructor.
      *
      * We need to override this as due to _paths we need deep copy..
      */
    VFS(const VFS &a) {
      _paths.setlen(a._paths.len);
      for(size_t i=0;i<a._paths.len;i++)
        _paths[i]=new alp::string(*a._paths[i]);
    }

    ~VFS();
    
    /** Add a path to the list of base directories */
    void addPath(const alp::string &path);

    /** Attempt to locate a given file.
      *
      * \return Resolved absolute path or empty string if the file was not 
      * found anywhere.
      */
    alp::string locate(const alp::string &fn_in);

};

#endif
