#ifndef RISCV_LUT_COMPILER_OPTIONS_H
#define RISCV_LUT_COMPILER_OPTIONS_H

struct options_t {
  
  // options go here.
  
  
  int fInputIntermediate;

  int fOutputIntermediate;
  int fOutputAssembly;

  alp::string fnArch;
  alp::string lutName;
  alp::string outputName;

  options_t() : 
    fInputIntermediate(0),
    fOutputIntermediate(0),
    fOutputAssembly(0)
    // strings initialize themselves
    {
  }

  /** Prints an overview of available options on the file specified.
    *
    * This is used to generate the help output of the lut compiler tool.
    */
  static void print(FILE *f);

  /** Parses a command line argument list.
    *
    * This method expects the command part of the command line to have been
    * filtered already, starting parsing at the *first* argument.
    *
    * \param argn Number of arguments to parse.
    * \param argv Array of arguments to parse, having at lesat argn
    * null-terminated string elements.
    * \return 0 if the command line was parsed successfully, 1 if an error
    * occurred and the program should be terminated with error condition and
    * 2 if the program should be terminated normally (e.g. if a --help switch 
    * was handled)
    */
  int parseCommandLine(int argn, const char **argv);

};

#endif
