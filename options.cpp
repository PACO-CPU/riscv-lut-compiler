#include "options.h"
#include "error.h"
#include <stdio.h>
#include <string.h>

void options_t::print(FILE *f) {
  fprintf(f,
    "riscv-lut-compiler [options] input-file\n"
    "  translates a C/C++ function annotated with keywords into a Lookup \n"
    "  table configuration.\n"
    "  This is outputted either as an intermediate format (that can be read\n"
    "  again) or a block of LUT configuration words as an assembly code file.\n"
    "options:\n"
    "  -h|--help\n"
    "    Print this help text and exit normally.\n"
    "  -i|--intermediate\n"
    "    Output an intermediate representation of the generated LUT instead \n"
    "    of building its bitstream.\n"
    "  -C|--ouptut-c\n"
    "    Output C code of the final bitstream instead of compiling it \n"
    "    to ELF.\n"
    "  -c|--compile\n"
    "    Input an intermediate format instead of the input format.\n"
    "  --arch <file>\n"
    "    Specify an architecture file to use as parameters of the physical \n"
    "    LUT to be generated.\n"
    "  -n|--name <name>\n"
    "    Set the identifier of the Lookup table. This overrides any names \n"
    "    defined in input files.\n"
    "  -o|--output <name>\n"
    "    Override the output file name. By default it is generated from the\n"
    "    Lookup table identifier and the appropriate extension of the output\n"
    "    format (.s for assembly, .o for ELF, .lut for intermediate)\n"
    "  -w|--weights-test\n"
    "    Input a weights file and sample it into a .dat file, used for \n"
    "    weight distributions.\n"
    "  --weight-steps <number>\n"
    "    set the maximum number of samples done when performing a \n"
    "    weights test. default: %i\n",
    Default_maxWeightSteps
    );
}


int options_t::parseCommandLine(int argn, const char **argv) {
  enum state_t {
    Idle,
    Name,
    Output,
    Arch,
    WeightSteps
  };
  state_t state=Idle;

  for(int i=0;i<argn;i++) switch(state) {
    #define SWITCH(id_short,id_long) ( \
      (strcmp(id_short,argv[i])==0) || \
      (strcmp(id_long,argv[i])==0) )
    #define LSWITCH(id_long) \
      (strcmp(id_long,argv[i])==0)
    case Idle:
      if (argv[i][0]=='-') {
        if (SWITCH("-i","--intermediate")) fOutputIntermediate=1;
        else if (SWITCH("-C","--output-c")) fOutputC=1;
        else if (SWITCH("-c","--compile")) fInputIntermediate=1;
        else if (SWITCH("-w","--weights-test")) fInputWeights=1;
        else if (SWITCH("-n","--name")) state=Name;
        else if (SWITCH("-o","--output")) state=Output;
        else if (LSWITCH("--arch")) state=Arch;
        else if (LSWITCH("--weight-steps")) state=WeightSteps;
        else if (SWITCH("-h","--help")) {
          print(stdout);
          return 2;
        } else {
          throw CommandLineError(CommandLineError::UnknownSwitch,argv[i]);
        }
      } else {
        if (fnInput.len>0) {
          throw CommandLineError(CommandLineError::StrayArgument,argv[i]);
        }
        fnInput=argv[i];
      }
      break;
    case Name:
      state=Idle;
      lutName=argv[i];
      break;
    case Output:
      state=Idle;
      outputName=argv[i];
      break;
    case Arch:
      state=Idle;
      fnArch=argv[i];
      arch.parseFile(fnArch.ptr);
      break;
    case WeightSteps:
      state=Idle;
      maxWeightSteps=atol(argv[i]);
      if (maxWeightSteps<=0)
        throw CommandLineError(
          CommandLineError::Semantics,
          "positive number expected for --weight-steps");
      break;

    #undef SWITCH
    #undef LSWITCH
  }

  switch(state) {
    #define ERRSTATE(s,n) \
    case s: \
      throw CommandLineError(CommandLineError::MissingArgument,n);

    ERRSTATE(Output,"--output")
    ERRSTATE(Name,"--name")
    ERRSTATE(Arch,"--arch")
    ERRSTATE(WeightSteps,"--weight-steps")

    default: break;

    #undef ERRSTATE
  }

  if (fnInput.len<1) 
    throw CommandLineError(
      CommandLineError::Semantics,"no input file specified");

  if (fOutputIntermediate && fOutputC)
    throw CommandLineError(
      CommandLineError::Semantics,"cannot specify -C and -i together");

  if (fInputIntermediate && fInputWeights)
    throw CommandLineError(
      CommandLineError::Semantics,"cannot specify -c and -w together");


  return 0;
}

void options_t::computeOutputName() {
  if (outputName.len>0) return;
  
  if (!fInputWeights && (lutName.len>0)) {
    outputName=lutName;
  } else {
    outputName=fnInput;
    for(ssize_t i=(ssize_t)fnInput.len-1;i>-1;i--)
      if (outputName.ptr[i]=='.') {
        outputName.resize(i); 
        break;
      } else if (outputName.ptr[i]=='/') {
        break;
      }
  }

  if (fInputWeights) {
    outputName+=".dat";
  } else if (fOutputIntermediate) {
    outputName+=".lut";
  } else if (fOutputC) {
    outputName+=".c";
  } else {
    outputName+=".o";
  }
}

