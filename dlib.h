#ifndef RISCV_LUT_COMPILER_DLIB_H
#define RISCV_LUT_COMPILER_DLIB_H

struct dynamic_library_t;

dynamic_library_t *dlib_open(const char *fn);
void *dlib_lookup(dynamic_library_t *lib, const char *name);
void dlib_close(dynamic_library_t *lib);

#endif
