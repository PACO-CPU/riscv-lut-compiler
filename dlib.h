/** \file dlib.h
  * \brief Defines functions for cross-platform loading of dynamic libraries.
  *
  * This is used to load the target function after it was compiled into a
  * shared object.
  *
  * Working with such libraries is handled by an opaque type, dynamic_library_t,
  * representing such a library. It is opened with dlib_open, contained 
  * functions can be retrieved with dlib_lookup and after the library is
  * no longer needed it is unloaded with dlib_close.
  *
  */
#ifndef RISCV_LUT_COMPILER_DLIB_H
#define RISCV_LUT_COMPILER_DLIB_H

/** Opaque type representing a library */
struct dynamic_library_t;

/** Opens a library file and returns an instance of the dynamic_library_t
  * representing it.
  *
  * If unsuccessful, NULL is returned. Otherwise the result is a valid
  * instance for use with dlib_lookup until dlib_close was called with it.
  */
dynamic_library_t *dlib_open(const char *fn);

/** Looks up a symbol from a loaded library as returned by a previous call
  * to dlib_open.. */
void *dlib_lookup(dynamic_library_t *lib, const char *name);

/** Closes a library previously loaded with dlib_open. */
void dlib_close(dynamic_library_t *lib);

#endif
