#include "weights.h"
#include "lut.h"
#include "segment.h"
#include "error.h"
#include "keyvalue.h"
#include "options.h"

static int run_weights_test(options_t &options);
int main(int argn, char **argv);


static int run_weights_test(options_t &options) {
  WeightsTable *tbl=new WeightsTable;
  FILE *fOutput;

  try {
    tbl->parseWeightsFile(options.fnInput.ptr);
  } catch(FileIOException &e) {
    fprintf(
      stderr,"\x1b[31;1mError loading weights file: %s\x1b[30;0m\n",
      e.what());
    return 1;
  } catch(SyntaxError &e) {
    fprintf(
      stderr,"\x1b[31;1mError parsing weights file %s: %s\x1b[30;0m\n",
      options.fnInput.ptr,e.what());
    return 1;
  }
  
  options.computeOutputName();
  fOutput=fopen(options.outputName.ptr,"wb");
  if (!fOutput) {
    fprintf(
      stderr,"\x1b[31;1mError opening output file: %s\x1b[30;0m\n",
      FileIOException(options.outputName).what());
    return 1;
  }

  double min=tbl->first();
  double max=tbl->last();
  double step= 
    (((max-min)>options.maxWeightSteps) || !tbl->isAllIntegers())
    ? (max-min)/(double)options.maxWeightSteps
    : 1;
  seg_data_t v;
  

  if (tbl->isAllIntegers()) {
    for(double p=min;p<=max;p+=step) {
      int64_t pi=(int64_t)p;
      tbl->evaluate(pi,v);
      switch(v.kind) {
        case seg_data_t::Double: 
          fprintf(fOutput,"%li\t%f\n",pi,v.data_f); 
          break;
        case seg_data_t::Integer: 
          fprintf(fOutput,"%li\t%li\n",pi,v.data_i); 
          break;
      }
    }
  } else {
    for(double p=min;p<=max;p+=step) {
      tbl->evaluate(p,v);
      switch(v.kind) {
        case seg_data_t::Double: 
          fprintf(fOutput,"%f\t%f\n",p,v.data_f); 
          break;
        case seg_data_t::Integer: 
          fprintf(fOutput,"%f\t%li\n",p,v.data_i); 
          break;
      }
    }

  }

  fclose(fOutput);
  delete tbl;

  return 0;

}


int main(int argn, char **argv) {
  options_t options;

  try {
    int res;
    switch((res=options.parseCommandLine(argn-1,(const char**)(argv+1)))) {
      case 2: 
        exit(0);

      default:
        break;
    }
  } catch(CommandLineError &e) {
    options.print(stderr);
    fprintf(stderr,"\x1b[31;1mERROR: %s\x1b[30;0m\n",e.what());
    exit(1);
  }

  if (options.fInputWeights) {
    return run_weights_test(options);
  }

  // todo: implement tool flow for lut compilation
  return 0;
}
