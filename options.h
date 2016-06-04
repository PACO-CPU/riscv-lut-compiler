#ifndef RISCV_LUT_COMPILER_OPTIONS_H
#define RISCV_LUT_COMPILER_OPTIONS_H
#include "arch-config.h"
#include <alpha/alpha.h>
#include <stdio.h>
struct options_t {
  enum {
    Default_maxWeightSteps = 1000,
  };
  // options go here.
  
  
  int fInputIntermediate;
  int fInputWeights;

  int fOutputIntermediate;
  int fOutputC;
  
  int maxWeightSteps;
  
  alp::string fnInput;
  alp::string fnArch;
  alp::string lutName;
  alp::string outputName;

  arch_config_t arch;

  options_t() : 
    fInputIntermediate(0),
    fInputWeights(0),
    fOutputIntermediate(0),
    fOutputC(0),
    maxWeightSteps(Default_maxWeightSteps)
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
    * 
    * \throw CommandLineError The command line is invalid.
    */
  int parseCommandLine(int argn, const char **argv);

  void computeOutputName();

};

#endif
